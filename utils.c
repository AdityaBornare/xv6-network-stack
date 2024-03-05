#include "types.h"
#include "defs.h"

void delay(uint milliseconds) {
  uint start_ticks = ticks;

  while ((ticks - start_ticks) < milliseconds) {
    // Wait until desired number of ticks has passed
  }
}

