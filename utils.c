#include "types.h"
#include "defs.h"

void delay(uint numticks) {
  uint end_ticks = ticks + numticks; // 10ms timer interrupt

  while (ticks < end_ticks) {
    // Wait until desired number of ticks has passed
  }
}
