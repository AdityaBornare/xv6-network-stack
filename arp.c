#include "types.h"
#include "defs.h"
#include "spinlock.h"
#include "arp.h"
#include "ether.h"
#include "ip.h"

#define ARP_CACHE_SIZE 10
#define ARP_TTL_TICKS 20 * 60 * 100

struct { 
  struct spinlock lk;
  struct arp_entry cache[ARP_CACHE_SIZE];
} arp_cache;

void arpinit() {
  initlock(&arp_cache.lk, "arp cache");
}

// call only when the arp cache lock is held
int arp_alloc() {
  for(int i = 0; i < ARP_CACHE_SIZE; i++) {
    // empty block
    if(arp_cache.cache[i].status == EMPTY)
      return i;

    // timed out entry
    if(arp_cache.cache[i].status == USED && arp_cache.cache[i].valid_until < ticks) {
      arp_cache.cache[i].status = EMPTY;
      return i;
    }
  }

  // envict oldest entry
  uint minticks = 0xffffffff;
  int i_min = -1;
  for(int i = 0; i < ARP_CACHE_SIZE; i++) {
    if(arp_cache.cache[i].status != REQUESTED && arp_cache.cache[i].valid_until < minticks) {
      minticks = arp_cache.cache[i].valid_until;
      i_min = i;
    }
  }
  if(i_min == -1)
    panic("arp cache full");
  arp_cache.cache[i_min].status = EMPTY;
  return i_min;
}

int arp_search(uint ip) {
  for(int i = 0; i < ARP_CACHE_SIZE; i++) {
    if(arp_cache.cache[i].ip != ip)
      continue;

    if(arp_cache.cache[i].status == USED && ticks <= arp_cache.cache[i].valid_until) // found in cache
      return i;
  }
  return -1;
}

void arp_request(uint ip) {
  struct arp_packet ap;
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
  struct arp_packet* req = (struct arp_packet*) arp_pkt;
  if(htonl(req->target_paddr) != MY_IP)
    return;
  struct arp_packet rep;
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
  if((ip & NETMASK) != (MY_IP & NETMASK))
    ip = GATEWAY;
  int i;

  acquire(&arp_cache.lk);

  // search in cache
  i = arp_search(ip);
  if(i != -1) {
    release(&arp_cache.lk);
    return arp_cache.cache[i].mac;
  }

  // check if already requested
  for(i = 0; i < ARP_CACHE_SIZE; i++) {
    if(arp_cache.cache[i].ip == ip && arp_cache.cache[i].status == REQUESTED) {
      // already requested
      sleep(&arp_cache.cache[i], &arp_cache.lk);
      release(&arp_cache.lk);
      return arp_cache.cache[i].mac;
    }
  }
  
  // not found in cache, send request
  i = arp_alloc();
  arp_cache.cache[i].ip = ip;
  arp_cache.cache[i].status = REQUESTED;
  arp_request(ip);
  sleep(&arp_cache.cache[i], &arp_cache.lk);
  release(&arp_cache.lk);
  return arp_cache.cache[i].mac;
}

void arp_add(uint ip, uchar *mac) {
  int i;
  int requested = 0;
  acquire(&arp_cache.lk);

  i = arp_search(ip);
  if(i != -1) {
    // already present in cache
    release(&arp_cache.lk);
    return;
  }

  for(i = 0; i < ARP_CACHE_SIZE; i++) {
    if(arp_cache.cache[i].ip == ip && arp_cache.cache[i].status == REQUESTED) {
      // already requested
      requested = 1;
      break;
    }
  }

  if(!requested) // allocate new entry
    i = arp_alloc();

  arp_cache.cache[i].ip = ip;
  memmove(arp_cache.cache[i].mac, mac, HADDR_SIZE);
  arp_cache.cache[i].valid_until = ticks + ARP_TTL_TICKS;
  arp_cache.cache[i].status = USED;

  if(requested)
    wakeup(&arp_cache.cache[i]);

  release(&arp_cache.lk);
  return;
}

void arp_receive(void *arp_pkt) {
  struct arp_packet *ap = (struct arp_packet*) arp_pkt;
  if(htons(ap->op) == ARP_OP_REQUEST)
    arp_reply(arp_pkt);

  else if(htons(ap->op) == ARP_OP_REPLY) {
    uint ip = htonl(ap->sender_paddr);
    int i;

    acquire(&arp_cache.lk);

    for(i = 0; i < ARP_CACHE_SIZE; i++) {
      if(arp_cache.cache[i].ip == ip && arp_cache.cache[i].status == REQUESTED)
        break;
    }

    if(i == ARP_CACHE_SIZE) {
      release(&arp_cache.lk);
      return;
    }

    memmove(arp_cache.cache[i].mac, ap->sender_haddr, HADDR_SIZE);
    arp_cache.cache[i].valid_until = ticks + ARP_TTL_TICKS;
    arp_cache.cache[i].status = USED;
    wakeup(&arp_cache.cache[i]);
    release(&arp_cache.lk);
  }
}
