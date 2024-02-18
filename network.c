#include "types.h"
#include "defs.h"
#include "network.h"

ushort id = 0;

void network_init(){

  // Initialize network layer, setup routing tables, etc.

}

void network_receive(void* ip_dgram, int dsize){
  
  // Extract the network layer packet from the Ethernet frame

  // Process the received packet
  // Perform routing, packet validation, and other network layer tasks

  // Pass the transport payload to the transport layer (UDP/TCP)
  // The transport layer will handle the headers internally

}

void network_send(uchar protocol, void* buffer, uchar *src_ip, uchar *dst_ip, int size){
  ip_packet pkt;
  pkt.ip_hdr.version_ihl = (4 << 4) | MINIMUM_IHL;
  pkt.ip_hdr.tos = 0;
  cprintf("hdr len = %d, size = %d\n", HDR_SIZE, size);
  cprintf("htons = %d\n", HDR_SIZE + size);
  pkt.ip_hdr.tlen = htons(HDR_SIZE + size);
  pkt.ip_hdr.id = htons(id++);
  pkt.ip_hdr.flags_offset = htons(0x4000);
  pkt.ip_hdr.ttl = INITIAL_TTL;
  pkt.ip_hdr.protocol = protocol;
  memmove(pkt.ip_hdr.src_ip, src_ip, IP_ADDR_SIZE);
  memmove(pkt.ip_hdr.dst_ip, dst_ip, IP_ADDR_SIZE);
  pkt.ip_hdr.checksum = 0;

  ushort *p = (ushort*) &pkt.ip_hdr;
  ushort sum = 0;
  for(int i = 0; i < HDR_SIZE / 2; i++) 
    sum += p[i];
  pkt.ip_hdr.checksum = htons(~sum);

  memmove(pkt.transport_payload, buffer, size);
  
  uchar destMAC[] = {0x12, 0x12, 0x12, 0x12, 0x12, 0x12};
  ether_send(destMAC, 0x0800, &pkt, HDR_SIZE + size);
}
