#include "types.h"
#include "defs.h"

ushort htons(ushort n) {
  ushort a = 0;
  ushort b;
  int bytes = sizeof(n);
  for(int i = 0; i < bytes; i++) {
    b = (n >> (i * 8)) & 0xff;
    a |= b << (8 * (bytes - i - 1));
  }
  return a;
}

uint htonl(uint n) {
  uint a = 0;
  uint b;
  int bytes = sizeof(n);
  for(int i = 0; i < bytes; i++) {
    b = (n >> (i * 8)) & 0xff;
    a |= b << (8 * (bytes - i - 1));
  }
  return a;
}
