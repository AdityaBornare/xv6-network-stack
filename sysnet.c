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

  arp_resolve(GATEWAY);

  return 0;
}

int
sys_ifconfig(void)
{
  cprintf("IP: %d.%d.%d.%d\n", (MY_IP >> 24) & 0xFF, (MY_IP >> 16) & 0xFF, (MY_IP >> 8) & 0xFF, MY_IP & 0xFF);
  cprintf("Netmask: %d.%d.%d.%d\n", (NETMASK >> 24) & 0xFF, (NETMASK >> 16) & 0xFF, (NETMASK >> 8) & 0xFF, NETMASK & 0xFF);
  cprintf("Gateway: %d.%d.%d.%d\n", (GATEWAY >> 24) & 0xFF, (GATEWAY >> 16) & 0xFF, (GATEWAY >> 8) & 0xFF, GATEWAY & 0xFF);
  return 0;
}

int
sys_socket(void)
{
  return 0;
}

int
sys_bind(void)
{
  return 0;
}

int
sys_listen(void)
{
  return 0;
}

int
sys_connect(void)
{
  return 0;
}

