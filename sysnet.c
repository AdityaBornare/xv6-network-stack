#include "types.h"
#include "defs.h"
#include "icmp.h"

extern int icmp_echo_reply_received;
extern struct icmp_reply_packet icmp_reply_pkt_info;

int
sys_ifset(void)
{
  uint ip, netmask, gateway;

  if(argint(0, (int*)&ip) < 0 || argint(1, (int*)&netmask) < 0 || argint(2, (int*)&gateway) < 0)
    return -1;
  MYIP = ip;
  NETMASK = netmask;
  GATEWAY = gateway;

  arp_resolve(GATEWAY);

  return 0;
}

int
sys_ifconfig(void)
{
  cprintf("IP: %d.%d.%d.%d\n", (MYIP >> 24) & 0xFF, (MYIP >> 16) & 0xFF, (MYIP >> 8) & 0xFF, MYIP & 0xFF);
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
  uint port;

  if(argint(0, &sockfd) < 0 || argint(1, (int*)&addr) < 0 || argint(2, (int*)&port) < 0)
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
  int sockfd;
  uint dst_addr;
  uint dst_port;

  if(argint(0, &sockfd) < 0 || argint(1, (int*)&dst_addr) < 0 || argint(2, (int*)&dst_port) < 0)
    return -1;
  return connect(sockfd, dst_addr, dst_port);
}

int
sys_accept(void)
{
  int sockfd;
  if(argint(0, &sockfd) < 0)
    return -1;
  return accept(sockfd);
}

int
sys_test(void)
{
  // char payload[] = "test";
  uint dst_ip;

  if(argint(0, (int*)&dst_ip) < 0)
    return -1;

  // ip_send(6, (uchar*)payload, MY_IP, dst_ip, sizeof(payload));
  // tcp_send(8888, 8888, dst_ip, 1, 1, 0, 20, (void*)payload, sizeof(payload));

  return 0;
}

int
sys_get_icmp_echo_reply_status(void)
{
  int status = icmp_echo_reply_received;
  icmp_echo_reply_received = 0; // Reset the value
  return status;
}

int
sys_get_icmp_echo_reply_packet(void) 
{
  int srcip = htonl(icmp_reply_pkt_info.src_ip);
  cprintf("%d bytes from %d.%d.%d.%d: icmp_seq=%d ",icmp_reply_pkt_info.size,srcip & 0xFF,(srcip >> 8) & 0xFF,(srcip >> 16) & 0xFF,(srcip >> 24) & 0xFF,icmp_reply_pkt_info.seq_no);
  return 0;
}

int
sys_icmp_send_echo_request(void)
{
  uint dst_ip;
  uint seq_no;

  if (argint(0, (int*)&dst_ip) < 0)
    return -1;

  if (argint(1, (int*)&seq_no) < 0)
    return -1;

  icmp_send_echo_request(dst_ip, seq_no);
  return 0;
}
