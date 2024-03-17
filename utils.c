#include "types.h"
#include "defs.h"

void delay(uint seconds) {
  uint end_ticks = ticks + seconds * 100; // assuming 10ms timer interrupt

  while (ticks < end_ticks) {
    // Wait until desired number of ticks has passed
  }
}
