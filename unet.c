#include "types.h"
#include "user.h"

uint inet_addr(char ip_str[]) {
  int byte = 0;
  int ip = 0;
  int i = 0;

  while(ip_str[i] != 0) {
    if(ip_str[i] == '.') {
      ip = ip << 8;
      ip |= byte;
      byte = 0;
    }
    else {
      byte = byte * 10 + (ip_str[i] - '0');
    }
    i++;
  }
  ip = ip << 8;
  ip |= byte;
  return ip;
}