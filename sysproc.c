#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "ether.h"

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
  ether_pack packet;

  // Set destination MAC address
  // Example: b0:dc:ef:bf:be:4f
  unsigned char destMAC[] = {0xb0, 0xdc, 0xef, 0xbf, 0xbe, 0x4f};
  memmove(packet.dst, destMAC, sizeof(destMAC));

  // Set source MAC address
  // 52:54:98:76:54:32
  unsigned char srcMAC[] = {0x52, 0x54, 0x98, 0x76, 0x54, 0x32};
  memmove(packet.src, srcMAC, sizeof(srcMAC));

  // Set EtherType (e.g., 0x0800 for IPv4)
  packet.ethertype = 0x0800;

  // Set data payload
  char payload[] = "Hello, this is a test packet!";
  memmove(packet.data, payload, sizeof(payload));

  // Send the Ethernet packet using rtl8139_send
  rtl8139_send((void*)&packet, sizeof(packet));

  return 0;
}
