#include "types.h"
#include "defs.h"

int
sys_ifset(void)
{
  uint ip, netmask, gateway;

  if(argint(0, (int*)&ip) < 0 || argint(1, (int*)&netmask) < 0 || argint(2, (int*)&gateway) < 0)
    return -1;
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
  int type;
  if(argint(0, &type) < 0)
    return -1;
  return socket(type);
}

int
sys_bind(void)
{
  int sockfd;
  uint addr;
  int port;

  if(argint(0, &sockfd) < 0 || argint(1, (int*)&addr) < 0 || argint(2, &port) < 0)
    return -1;
  return bind(sockfd, addr, port);
}

int
sys_listen(void)
{
  int sockfd;
  if(argint(0, &sockfd) < 0)
    return -1;
  return listen(sockfd);
}

int
sys_connect(void)
{
  return 0;
}

