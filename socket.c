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
#include "socket.h"

struct port {
  int pid;
  struct socket *socket;
};
struct port ports[NPORTS];

void socketinit() {
  for(int i = 0; i < NPORTS; i++)
    ports[i].pid = -1;
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
  if(ports[port].pid != -1)
    return -1;
  if(addr != MY_IP)
    return -1;
  if(sockfd < 0 || sockfd >= NOFILE || (sockfile = myproc()->ofile[sockfd]) == 0)
    return -1;

  if(sockfile->type != FD_SOCKET || (socket = sockfile->socket) == 0)
    return -1;

  socket->addr = addr;
  socket->port = port;
  ports[port].pid = myproc()->pid;
  ports[port].socket = socket;
  socket->status |= SOCKET_BOUND;
  return 0;
}

int listen(int sockfd) {
  struct file *sockfile;
  struct socket *socket;

  if(sockfd < 0 || sockfd >= NOFILE || (sockfile = myproc()->ofile[sockfd]) == 0)
    return -1;

  if(sockfile->type != FD_SOCKET || (socket = sockfile->socket) == 0)
    return -1;

  if((socket->status & SOCKET_BOUND) == 0)
    return -1;

  socket->status |= SOCKET_LISTENING;
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
  struct socket *socket;

  if(sockfd < 0 || sockfd >= NOFILE || (sockfile = myproc()->ofile[sockfd]) == 0)
    return -1;

  if(sockfile->type != FD_SOCKET || (socket = sockfile->socket) == 0)
    return -1;
  
  if((socket->status & SOCKET_BOUND) == 0 || (socket->status & SOCKET_LISTENING) == 0)
    return -1;

  // check waitqueue for pending requests 
  // sleep if no waitqueue empty
  // dequeue the requests on FIFO basis
  // save info from request to tcon
  // send syn ack, update tcon
  // wait for reply (sleep)
  // find reply in buffer, extract, update tcon
  // allocate new fd - refer socket syscall
  // bind new struct socket to same address and port as sockfd
  // set SOCKET_BOUND and SOCNET_CONNECTED in status of new fd
  // return new fd
  return 0;
}
