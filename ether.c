#include "types.h"
#include "defs.h"
#include "x86.h"
#include "ether.h"

void ether_send(unsigned char* destMAC, unsigned char* srcMAC, unsigned short type, unsigned char* payload, uint plen){
  ether_hdr *header;
  uint flen;

  cprintf("size of playload : %d\n", plen);
  unsigned char frame[ETHERNET_FRAME_SIZE_MAX];

  header = (ether_hdr*)frame;

  // Set destination MAC address
  // Example: b0:dc:ef:bf:be:4f
  memmove(header->dst, destMAC, MAC_SIZE);

  // Set source MAC address
  // 52:54:98:76:54:32
  memmove(header->src, srcMAC, MAC_SIZE);

  // Set the type
  header->type = type;

  //Set the payload
  memmove(header + 1, payload, plen);

  flen = ETHERNET_HDR_SIZE + plen;

  cprintf("size of frame : %d\n",flen);

  // Send the Ethernet frame using rtl8139_send
  rtl8139_send((void*)&frame, flen);
}
