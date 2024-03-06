#include "types.h"
#include "defs.h"

void delay(uint milliseconds) { 
  uint end_ticks = ticks + milliseconds;

  while (ticks < end_ticks) {
    // Wait until desired number of ticks has passed
  }
}

