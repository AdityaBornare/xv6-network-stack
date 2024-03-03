#include "types.h"
#include "defs.h"
#include "x86.h"
#include "ether.h"

// mac address - initialized in nicinit
char MAC[MAC_SIZE];

// prepare and send ethernet frame
void ether_send(uchar* destMAC, ushort type, void* payload, uint plen){
  if(plen > ETHERNET_PAYLOAD_SIZE_MAX) {
    cprintf("maximum payload size exceeded!\n");
    return;
  }
  ether_hdr *header;
  uint flen;

  cprintf("size of payload : %d\n", plen);
  uchar frame[ETHERNET_FRAME_SIZE_MAX];

  header = (ether_hdr*)frame;

  // Set destination MAC address
  // Example: b0:dc:ef:bf:be:4f
  memmove(header->dst, destMAC, MAC_SIZE);

  // Set source MAC address
  // 52:54:98:76:54:32
  memmove(header->src, MAC, MAC_SIZE);

  // Set the type
  header->type = htons(type);

  //Set the payload
  memmove(header + 1, payload, plen);

  flen = ETHERNET_HDR_SIZE + plen;

  cprintf("size of frame : %d\n",flen);

  // Send the Ethernet frame using rtl8139_send
  rtl8139_send((void*)&frame, flen);
}

// extract playload from ethernet frame, pkt_size = frame_size - 4  (exclude CRC)
void ether_receive(void *eth_frame, int pkt_size) {
  ether_hdr *header = (ether_hdr*) eth_frame;
  char *payload = (char*) (header + 1);
  ushort type = htons(header->type); 
  // char data[pkt_size + 1];
  // int i;
  /*
  for(i = 0; i < pkt_size - ETHERNET_HDR_SIZE; i++)
    data[i] = payload[i];
  data[i] = 0;
  */
  cprintf("%x\n", type);
  if(type == ETHERNET_TYPE_IP)
    ip_receive(payload, pkt_size - ETHERNET_HDR_SIZE);
  else if(type == ETHERNET_TYPE_ARP)
    arp_receive(payload);
  else
    return;
}
