#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "icmp.h" 

extern int icmp_echo_reply_received;
extern struct icmp_reply_packet icmp_reply_pkt_info;

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_test(void)
{
  char payload[] = "test";
  uint dst_ip;

  if(argint(0, (int*)&dst_ip) < 0)
    return -1;

  ip_send(6, (uchar*)payload, MY_IP, dst_ip, sizeof(payload));
  tcp_send(8888, 8888, dst_ip, 1, 1, 0, 20, (void*)payload, sizeof(payload));

  return 0;
}

int
sys_get_icmp_echo_reply_status(void)
{
  int status = icmp_echo_reply_received;
  icmp_echo_reply_received = 0; // Reset the value
  return status;
}

int 
sys_get_icmp_echo_reply_packet(void) {
  int srcip = htonl(icmp_reply_pkt_info.src_ip);
  cprintf("%d bytes from %d.%d.%d.%d: icmp_seq=%d ",icmp_reply_pkt_info.size,srcip & 0xFF,(srcip >> 8) & 0xFF,(srcip >> 16) & 0xFF,(srcip >> 24) & 0xFF,icmp_reply_pkt_info.seq_no);
  return 0;
}



int
sys_icmp_send_echo_request(void)
{
  uint dst_ip;
  uint seq_no;

  if (argint(0, (int*)&dst_ip) < 0)
    return -1;
    
  if (argint(1, (int*)&seq_no) < 0)
    return -1;

  icmp_send_echo_request(dst_ip, seq_no);
  return 0;
}
