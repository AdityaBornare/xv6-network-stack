#include "types.h"
#include "user.h"
#include "defs.h"

int main() {
  
  printf(1, "IP Address: %d \n", MY_IP);
  printf(1, "Netmask: %d \n", NETMASK);
  printf(1, "Gateway: %d \n", GATEWAY);
  exit();
}