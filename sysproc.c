#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

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
  /* unsigned char destMAC[] = {0x52, 0x54, 0x98, 0x76, 0x54, 0x32};

  unsigned char srcMAC[] = {0x52, 0x54, 0x98, 0x76, 0x54, 0x33};

  unsigned short type = 0x0000;

  unsigned char payload[] = "Testing send";

  uint plen = sizeof(payload);
  cprintf("%d\n", plen);

  // Send the Ethernet packet
  ether_send(destMAC, srcMAC, type, payload, plen);
  */
  uchar playload[] = "test";
  uchar src_ip[] = {192, 168, 2, 1};
  uchar dst_ip[] = {192, 168, 1, 1};
  network_send(6, playload, src_ip, dst_ip, sizeof(playload));
  return 0;
}
