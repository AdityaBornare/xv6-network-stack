#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "mmu.h"
#include "proc.h"

void delay(uint n) {
  acquire(&tickslock);
  uint ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
}
