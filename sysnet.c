#include "types.h"
#include "defs.h"

int
sys_ifset(void)
{
  int ip, netmask, gateway;

  argint(0, &ip);
  argint(1, &netmask);
  argint(2, &gateway);

  MY_IP = ip;
  NETMASK = netmask;
  GATEWAY = gateway;

  return 0;
}