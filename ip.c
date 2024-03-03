#include "types.h"
#include "defs.h"
#include "ip.h"

ushort id = 0;
uint MY_IP = 0xc0a80202;

void ip_init(){

  // Initialize network layer, setup routing tables, etc.

}

void ip_receive(void* ip_dgram, int dsize){
  // Extract the network layer packet from the Ethernet frame
  ip_header* rx_ip_hdr = (ip_header*)ip_dgram;

  // validate checksum
  rx_ip_hdr->checksum = htons(rx_ip_hdr->checksum);
  ushort *p = (ushort*) rx_ip_hdr;
  ushort sum = 0;

  for(int i = 0; i < HDR_SIZE / 2; i++)
    sum += p[i];

  if (sum != 0xffff) {
    cprintf("Invalid checksum.");
    return;
  }

  // Pass the transport payload to the transport layer (UDP/TCP)
  // void* transport_payload = (void*)((uchar*)ip_dgram + (received_ip_hdr->version_ihl & 0x0F) * 4);

  // The transport layer will handle the headers internally
}

void ip_send(uchar protocol, void* buffer, uint src_ip, uint dst_ip, int size) {
  ip_packet pkt;
  pkt.ip_hdr.version_ihl = (4 << 4) | MINIMUM_IHL;
  pkt.ip_hdr.tos = 0;
  pkt.ip_hdr.tlen = htons(HDR_SIZE + size);
  pkt.ip_hdr.id = htons(id++);
  pkt.ip_hdr.flags_offset = htons(0x4000);
  pkt.ip_hdr.ttl = INITIAL_TTL;
  pkt.ip_hdr.protocol = protocol;
  pkt.ip_hdr.src_ip = htonl(src_ip);
  pkt.ip_hdr.dst_ip = htonl(dst_ip);
  pkt.ip_hdr.checksum = 0;

  ushort *p = (ushort*) &pkt.ip_hdr;
  ushort sum = 0;
  for(int i = 0; i < HDR_SIZE / 2; i++)
    sum += p[i];
  pkt.ip_hdr.checksum = htons(~sum);

  memmove(pkt.transport_payload, buffer, size);

  uchar *mac = arp_resolve(dst_ip);
  for(int i = 0; i < 6; i++) {
    cprintf("%x ", mac[i]);
  }
  cprintf("\n");
  /*
  if (dst_ip == 0xffffffff) {
    uchar destMAC[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    ether_send(destMAC, 0x0800, &pkt, HDR_SIZE + size);
  }
  else {
    uchar destMAC[] = {0x52, 0x54, 0x98, 0x76, 0x54, 0x33};
    ether_send(destMAC, 0x0800, &pkt, HDR_SIZE + size);
  }
  */
}
