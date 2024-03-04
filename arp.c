#include "types.h"
#include "defs.h"
#include "arp.h"
#include "ether.h"
#include "ip.h"

#define ARP_CACHE_SIZE 10
#define ARP_TTL_TICKS 20 * 60 * 100

arp_entry arp_cache[ARP_CACHE_SIZE];
uint request_pending = 0;

int search_cache(uint ip) {
  for(int i = 0; i < ARP_CACHE_SIZE; i++) {
    if(arp_cache[i].ip == ip && arp_cache[i].is_valid && ticks <= arp_cache[i].valid_until)
      return i;
  }
  return -1;
}

void update_cache() {
  for(int i = 0; i < ARP_CACHE_SIZE; i++) {
    if(arp_cache[i].is_valid == 1 && ticks > arp_cache[i].valid_until)
      arp_cache[i].is_valid = 0;
  }
}

void arp_request(uint ip) {
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
}

void arp_reply(void *arp_pkt) {
  arp_packet* req = (arp_packet*) arp_pkt;
  if(htonl(req->target_paddr) != MY_IP)
    return;
  arp_packet rep;
  rep.hwd_type = htons(ARP_HTYPE_ETHERNET);
  rep.prot_type = htons(ETHERNET_TYPE_IP);
  rep.hwd_length = MAC_SIZE;
  rep.prot_length = IP_ADDR_SIZE;
  rep.op = htons(ARP_OP_REPLY);
  memmove(rep.sender_haddr, MAC, MAC_SIZE);
  rep.sender_paddr = htonl(MY_IP);
  memmove(rep.target_haddr, req->sender_haddr, MAC_SIZE);
  rep.target_paddr = req->sender_paddr;

  ether_send(req->sender_haddr, ETHERNET_TYPE_ARP, (void*) &rep, sizeof(rep));
}

uchar *arp_resolve(uint ip) {
  int i;
  i = search_cache(ip);
  if(i != -1)
    return arp_cache[i].mac;

  request_pending = 1;
  arp_request(ip);
  while(request_pending == 1) {
    delay(10);
  }
  i = search_cache(ip);
  cprintf("i = %d\n", i);
  return arp_cache[i].mac;
}

void arp_receive(void *arp_pkt) {
  arp_packet* ap = (arp_packet*) arp_pkt;
  if(htons(ap->op) == ARP_OP_REQUEST)
    arp_reply(arp_pkt);
  else if(htons(ap->op) == ARP_OP_REPLY) {
    update_cache();
    for(int i = 0; i < ARP_CACHE_SIZE; i++) {
      if(!arp_cache[i].is_valid) {
        arp_cache[i].ip = htonl(ap->sender_paddr);
        memmove(arp_cache[i].mac, ap->sender_haddr, MAC_SIZE);
        arp_cache[i].is_valid = 1;
        arp_cache[i].valid_until = ticks + ARP_TTL_TICKS;
        break;
      }
    }
    request_pending = 0;
  }
}
