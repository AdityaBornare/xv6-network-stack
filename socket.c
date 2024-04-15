#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "mmu.h"
#include "proc.h"
#include "tcp.h"
#include "queue.h"
#include "socket.h"

struct port ports[NPORTS];
struct spinlock portlock;

void portinit() {
  for(int i = 0; i < NPORTS; i++)
    ports[i].pid = -1;
  initlock(&portlock, "ports");
}


// returns fd on success, -1 on error
int socket(int type) {
  int fd;
  struct file *f;
  struct socket *s;

  if((s = (struct socket*)kalloc()) == 0) {
    return -1;
  }

  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    kfree((char*)s);
    return -1;
  }

  memset((void*)s, 0, sizeof(struct socket));
  s->type = type;
  s->state = SOCKET_UNBOUND;

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
  struct socket *socket;

  if(port > NPORTS)
    return -1;
  if(addr != MY_IP)
    return -1;
  if(sockfd < 0 || sockfd >= NOFILE || (sockfile = myproc()->ofile[sockfd]) == 0)
    return -1;
  if(sockfile->type != FD_SOCKET || (socket = sockfile->socket) == 0)
    return -1;
  if(socket->state != SOCKET_UNBOUND)
    return -1;
  if(ports[port].pid != -1)
    return -1;
  
  acquire(&portlock);
  ports[port].pid = myproc()->pid;
  ports[port].socket = socket;
  release(&portlock);

  socket->addr = addr;
  socket->port = port;
  socket->state = SOCKET_BOUND;
  return 0;
}

int listen(int sockfd) {
  struct file *sockfile;
  struct socket *socket;

  if(sockfd < 0 || sockfd >= NOFILE || (sockfile = myproc()->ofile[sockfd]) == 0)
    return -1;
  if(sockfile->type != FD_SOCKET || (socket = sockfile->socket) == 0)
    return -1;
  if(socket->state != SOCKET_BOUND)
    return -1;

  initqueue(&(socket->waitqueue));
  socket->state = SOCKET_LISTENING;
  socket->buffer = kalloc();
  return 0;
}

int connect(int sockfd, uint dst_addr, ushort dst_port) {
  struct file *sockfile;
  struct socket *socket;

  if(sockfd < 0 || sockfd >= NOFILE || (sockfile = myproc()->ofile[sockfd]) == 0)
    return -1;

  if(sockfile->type != FD_SOCKET || (socket = sockfile->socket) == 0)
    return -1;

  // search for a free port and assign port and addr (MYIP)
  // set SOCKET_BOUND bit in status
  // send tcp connection request to given address, store info in tcon
  // wait for reply (sleep)
  // find reply at the start at buffer
  // store info from reply in tcon
  // send ack
  // set SOCKET_CONNECTED bit in status
  return 0;
}

int accept(int sockfd) {
  struct file *sockfile;
  struct socket *mysocket;

  if(sockfd < 0 || sockfd >= NOFILE || (sockfile = myproc()->ofile[sockfd]) == 0)
    return -1;

  if(sockfile->type != FD_SOCKET || (mysocket = sockfile->socket) == 0)
    return -1;
  
  if(mysocket->state != SOCKET_LISTENING)
    return -1;

  if(isqueueempty(mysocket->waitqueue)) {
    mysocket->state = SOCKET_ACCEPTING;
    sleepnolock(&ports[mysocket->port]);
  }

  mysocket->state = SOCKET_LISTENING;
  struct tcp_request *request = (struct tcp_request*) dequeue(&(mysocket->waitqueue));
  struct tcp_packet* tcp_req_packet = &request->request_packet;

  mysocket->tcon.dst_addr = request->client_ip;
  mysocket->tcon.dst_port = tcp_req_packet->header.src_port;
  mysocket->tcon.ack_received = tcp_req_packet->header.ack_num;
  mysocket->tcon.seq_received = tcp_req_packet->header.seq_num;

  struct tcp_mss_option mss;
  mss.kind = 2;
  mss.length = 4;
  mss.mss = MSS;

  // send syn ack, update tcon
  tcp_send(
    mysocket->port,
    mysocket->tcon.dst_port,
    mysocket->tcon.dst_addr,
    0,
    tcp_req_packet->header.seq_num + 1,
    TCP_FLAG_SYN | TCP_FLAG_ACK,
    TCP_HEADER_MIN_SIZE + mss.length,
    &mss,
    mss.length
  );

  mysocket->tcon.ack_sent = tcp_req_packet->header.ack_num;
  mysocket->tcon.seq_sent = tcp_req_packet->header.seq_num;
  
  // wait for ack (sleep)
  mysocket->state = SOCKET_WAITING_FOR_ACK;
  sleepnolock(&ports[mysocket->port]);

  // find reply in buffer, extract, update tcon
  struct tcp_packet* tcp_res_packet = (struct tcp_packet*) mysocket->buffer;
  mysocket->tcon.ack_received = tcp_res_packet->header.ack_num;
  mysocket->tcon.seq_received = tcp_res_packet->header.seq_num;
  
  int newfd = socket(TCP);
  bind(newfd,  mysocket->addr, mysocket->port);
  myproc()->ofile[newfd]->socket->buffer = kalloc();
  myproc()->ofile[newfd]->socket->state = SOCKET_CONNECTED;
  return newfd;
}
