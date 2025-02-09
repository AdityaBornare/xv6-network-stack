#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "mmu.h"
#include "proc.h"
#include "queue.h"
#include "tcp.h"
#include "socket.h"

struct port ports[NPORTS];
struct spinlock portlock;

struct socket sockets[NSOCKETS];
struct spinlock socklock;

void socketinit() {
  initlock(&socklock, "sockets");
} 

void portinit() {
  for(int i = 0; i < NPORTS; i++)
    ports[i].pid = -1;
  initlock(&portlock, "ports");
}

struct socket* socketalloc() {
  acquire(&socklock);
  for(int i = 0; i < NSOCKETS; i++){
    if(sockets[i].state == SOCKET_FREE) {    
      sockets[i].state = SOCKET_UNBOUND;
      release(&socklock);
      return &sockets[i];
    }
  }
  release(&socklock);
  return 0;
}

void socketfree(struct socket* s) {
  acquire(&socklock);
  s->state = SOCKET_FREE;
  release(&socklock);
}

// returns fd on success, -1 on error
int socket(int type) {
  int fd;
  struct file *f;
  struct socket *s;

  if((s = socketalloc()) == 0)
    return -1;  

  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    socketfree(s);
    return -1;
  }

  s->type = type;
  s->buffer = kalloc();

  f->type = FD_SOCKET;
  f->off = 0;
  f->readable = 1;
  f->writable = 1;
  f->socket = s;
  return fd;
}

// returns 0 on success, -1 on error
int bind(int sockfd, uint addr, ushort port) {
  struct file *sockfile;
  struct socket *s;

  if(port > NPORTS)
    return -1;
  if(addr != MYIP)
    return -1;
  if(sockfd < 0 || sockfd >= NOFILE || (sockfile = myproc()->ofile[sockfd]) == 0)
    return -1;
  if(sockfile->type != FD_SOCKET || (s = sockfile->socket) == 0)
    return -1;
  if(s->state != SOCKET_UNBOUND)
    return -1;
  if(ports[port].pid != -1)
    return -1;
  
  acquire(&portlock);
  ports[port].pid = myproc()->pid;
  ports[port].passive_socket = s;
  release(&portlock);

  s->addr = addr;
  s->port = port;
  s->state = SOCKET_BOUND;
  return 0;
}

int listen(int sockfd) {
  struct file *sockfile;
  struct socket *s;

  if(sockfd < 0 || sockfd >= NOFILE || (sockfile = myproc()->ofile[sockfd]) == 0)
    return -1;
  if(sockfile->type != FD_SOCKET || (s = sockfile->socket) == 0)
    return -1;
  if(s->state != SOCKET_BOUND)
    return -1;

  initqueue(&(s->waitqueue));
  s->tcon.state = TCP_LISTEN;
  return 0;
}

int connect(int sockfd, uint dst_addr, ushort dst_port) {
  struct file *sockfile;
  struct socket *s;

  if(sockfd < 0 || sockfd >= NOFILE || (sockfile = myproc()->ofile[sockfd]) == 0)
    return -1;

  if(sockfile->type != FD_SOCKET || (s = sockfile->socket) == 0)
    return -1;

  // search for a free port and assign port and addr (MYIP)
  int i;

  acquire(&portlock);
  for(i = 1024; i < NPORTS; i++) {
    if(ports[i].pid == -1) {
      ports[i].pid = myproc()->pid;
      ports[i].active_socket = s;
      // Assign the address (MYIP) to the socket
      s->addr = MYIP;
      // Assign the port to the socket
      s->port = i;
      // Set the socket state to SOCKET_BOUND
      s->state = SOCKET_BOUND; 
      break;
    }
  }
  release(&portlock);

  if(i == NPORTS) {
    // No free port available
    return -1;
  }

  // send tcp connection request to given address, store info in tcon
  s->tcon.dst_addr = dst_addr;
  s->tcon.dst_port = dst_port;

  tcp_send_syn(s);

  // wait for reply
  s->tcon.state = TCP_SYN_SENT;
  sleepnolock(&ports[s->port]);

  struct tcp_packet* tcp_reply_packet = (struct tcp_packet*) s->buffer;

  s->tcon.ack_received = htonl(tcp_reply_packet->header.ack_num);
  s->tcon.seq_received = htonl(tcp_reply_packet->header.seq_num);
  s->tcon.base_seq = s->tcon.ack_received;
  s->tcon.next_seq = s->tcon.ack_received;

  // send ack
  tcp_send_ack(s, s->tcon.seq_received + 1);
  initqueue(&s->tcon.window);
  s->tcon.state = TCP_ESTABLISHED;
  return 0;
}

int accept(int sockfd) {
  struct file *sockfile;
  struct socket *s;

  if(sockfd < 0 || sockfd >= NOFILE || (sockfile = myproc()->ofile[sockfd]) == 0)
    return -1;
  if(sockfile->type != FD_SOCKET || (s = sockfile->socket) == 0)
    return -1; 
  if(s->tcon.state != TCP_LISTEN)
    return -1;
  if(ports[s->port].active_socket != 0)
    return -1;

  if(isqueueempty(s->waitqueue)) {
    s->state = SOCKET_WAIT;
    sleepnolock(&ports[s->port]);
  }

  s->state = SOCKET_BOUND;
  struct tcp_request *request = (struct tcp_request*) dequeue(&(s->waitqueue));
  struct tcp_packet* tcp_req_packet = &request->request_packet;
  s->tcon.dst_addr = request->client_ip;
  s->tcon.dst_port = htons(tcp_req_packet->header.src_port);
  s->tcon.seq_received = htonl(tcp_req_packet->header.seq_num);

  tcp_send_synack(s);
 
  // wait for ack (sleep)
  s->tcon.state = TCP_SYNACK_SENT;
  sleepnolock(&ports[s->port]);

  // find reply in buffer, extract, update tcon
  // struct tcp_packet* tcp_res_packet = (struct tcp_packet*) s->buffer;
  // s->tcon.ack_received = htons(tcp_res_packet->header.ack_num);
  // s->tcon.seq_received = htons(tcp_res_packet->header.seq_num);
  s->tcon.state = TCP_LISTEN;
  
  int newfd = socket(TCP);
  struct socket *new_socket = myproc()->ofile[newfd]->socket;
  ports[s->port].active_socket = new_socket;
  new_socket->addr = s->addr;
  new_socket->port = s->port;
  new_socket->state = SOCKET_CONNECTED; 
  new_socket->tcon.dst_addr = s->tcon.dst_addr;
  new_socket->tcon.dst_port = s->tcon.dst_port;
  new_socket->tcon.ack_received = s->tcon.ack_received;
  new_socket->tcon.seq_received = s->tcon.seq_received;
  new_socket->tcon.base_seq = s->tcon.ack_received;
  new_socket->tcon.next_seq = s->tcon.ack_received;
  new_socket->tcon.ack_sent = s->tcon.ack_sent;
  initqueue(&new_socket->tcon.window);
  new_socket->tcon.state = TCP_ESTABLISHED;
  return newfd;
}

int socketwrite(struct socket *s, char *payload, int payload_size) {
  // check socket state
  if(s->tcon.state != TCP_ESTABLISHED)
    return -1;

  tcp_tx(s, payload, payload_size);
  return payload_size;
}

int socketread(struct socket *s, void *dst, int size) {
  if(s->tcon.state != TCP_ESTABLISHED)
    return -1;

  if(s->offset == s->end) {
    s->state = SOCKET_WAIT;
    sleepnolock(&ports[s->port]);
    s->state = SOCKET_CONNECTED;
  }
  int maxbytes = s->end - s->offset;
  int bytes = maxbytes >= size ? size : maxbytes;

  memmove(dst, s->buffer + s->offset, bytes);
  s->offset += bytes;
  return bytes;
}

void socketclose(struct socket *s) {
  int port = s->port;
  kfree(s->buffer);
  memset(s, 0, sizeof(struct socket));
  socketfree(s);

  acquire(&portlock);
  if(ports[port].active_socket == s) {
    ports[port].active_socket = 0;
    if(ports[port].passive_socket == 0)
      ports[port].pid = -1;
  }
  else if(ports[port].passive_socket == s) {
    ports[port].passive_socket = 0;
    ports[port].pid = -1;
  }
  release(&portlock);
}
