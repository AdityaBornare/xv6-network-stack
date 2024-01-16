#include "types.h"
#include "defs.h"
#include "x86.h"
#include "ether.h"

void send_ether_packet(unsigned char* destMAC,unsigned char* srcMAC,unsigned short ethertype,unsigned char* payload){
  ether_pack packet;
  
  // Set destination MAC address
  // Example: b0:dc:ef:bf:be:4f
  memmove(packet.dst, destMAC, sizeof(destMAC));
  
  // Set source MAC address
  // 52:54:98:76:54:32
  memmove(packet.src, srcMAC, sizeof(srcMAC));

  // Set EtherType 
  packet.ethertype = ethertype;

  // Set data payload
  memmove(packet.data, payload, sizeof(payload));
  
  // Send the Ethernet packet using rtl8139_send
  rtl8139_send((void*)&packet, sizeof(packet));
}
