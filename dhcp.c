#include "types.h"
#include "defs.h"
#include "dhcp.h"
#include "udp.h"

uint magic_cookie = 0x63825363;

void dhcp_discover() {
  udp_packet pkt;
  pkt.udp_hdr.src_port = htons(68);
  pkt.udp_hdr.dst_port = htons(67);

  dhcp_msg msg;
  msg.op = 1;
  msg.htype = 1;
  msg.hlen = 6;
  msg.hops = 0;
  msg.xid = htonl(0x3903F326);
  msg.secs = 0;
  msg.flags = 0;
  msg.ciaddr = 0;
  msg.yiaddr = 0;
  msg.siaddr = 0;
  msg.giaddr = 0;
  memmove(msg.chaddr, MAC, 6);
  memset(msg.chaddr + 6, 0, 202);
  msg.magic_cookie = htonl(magic_cookie);
  msg.type.code = DHCP_OPTION_TYPE;
  msg.type.len = 1;
  msg.type.type = DHCPDISCOVER;
  msg.end_code = 255;

  memmove(pkt.app_playload, &msg, sizeof(msg));
  memset(pkt.app_playload + sizeof(msg), 0, MAX_APP_PLAYLOAD_SIZE - sizeof(msg));
  ushort len = UDP_HEADER_LENGTH + sizeof(msg);
  pkt.udp_hdr.length = htons(len);

  pkt.udp_hdr.checksum = 0;
  ushort *p = (ushort*) &pkt;
  ushort sum = 0;
  int n = len % 2 == 0 ? len : len + 1;
  for(int i = 0; i < n / 2; i++)
    sum += p[i];
  pkt.udp_hdr.checksum = htons(~sum);
  
  uint src_ip = 0;
  uint dst_ip = inet_addr("255.255.255.255");
  network_send(17, (void*) &pkt, src_ip, dst_ip, len);
}
