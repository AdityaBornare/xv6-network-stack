#include "types.h"
#include "defs.h"
#include "arp.h"
#include "ether.h"
#include "ip.h"

char *arp_resolve(uint ip) {
  arp_packet ap;
  ap.hwd_type = htons(ARP_HTYPE_ETHERNET);
  ap.prot_type = htons(ETHERNET_TYPE_IP);
  ap.hwd_length = MAC_SIZE;
  ap.prot_length = IP_ADDR_SIZE;
  ap.op = htons(ARP_OP_REQUEST);
  memmove(ap.sender_haddr, MAC, MAC_SIZE);
  ap.sender_paddr = htonl(MY_IP);
  memset(ap.target_haddr, 0, MAC_SIZE);
  ap.target_paddr = htonl(ip);
  
  uchar dst_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  ether_send(dst_mac, ETHERNET_TYPE_ARP, (void*) &ap, sizeof(ap));
  return 0;
}
