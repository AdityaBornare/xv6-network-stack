#include "types.h"
#include "defs.h"
#include "ip.h"

#define IP_VERSION_4 4

ushort id = 0;
uint MY_IP = 0xc0a80202;
uint NETMASK = 0xffffff00;
uint GATEWAY = 0xc0a80201;

void ip_init(){

  // Initialize network layer, setup routing tables, etc.

}

void ip_receive(void* ip_dgram, int dsize){
  // Extract the network layer packet from the Ethernet frame
  ip_packet* rx_pkt = (ip_packet*) ip_dgram;

  // header checks
  if ((rx_pkt->ip_hdr.version_ihl >> 4) != IP_VERSION_4) {
    cprintf("Not an IPv4 packet.\n");
    return;
  }
  if (!rx_pkt->ip_hdr.ttl) {
    cprintf("Received IP packet was dead with TTL=0.\n");
    return;
  }
  // validate checksum
  if ((checksum(&rx_pkt->ip_hdr, IP_HDR_SIZE)) != 0) {
    cprintf("Invalid checksum\n");
    return;
  }

  // Check the protocol field in the IP header
  switch (rx_pkt->ip_hdr.protocol) {
    case IP_PROTOCOL_ICMP:
      // If the protocol is ICMP, call the icmp_receive function
      icmp_receive(rx_pkt->transport_payload, dsize - IP_HDR_SIZE, htonl(rx_pkt->ip_hdr.src_ip));
      break;

    default:
      char buf[dsize - IP_HDR_SIZE + 1];
      memmove(buf, rx_pkt->transport_payload, dsize - IP_HDR_SIZE);
      buf[dsize - IP_HDR_SIZE] = 0;
      cprintf("message = %s\n", buf);

      // Handle other protocols (TCP, UDP, etc.) as needed
  }
  
  // Pass the transport payload to the transport layer (UDP/TCP)
  // void* transport_payload = (void*)((uchar*)ip_dgram + (received_ip_hdr->version_ihl & 0x0F) * 4);

  // The transport layer will handle the headers internally
}

void ip_send(uchar protocol, void* buffer, uint src_ip, uint dst_ip, int size) {
  ip_packet pkt;
  pkt.ip_hdr.version_ihl = (4 << 4) | MINIMUM_IHL;
  pkt.ip_hdr.tos = 0;
  pkt.ip_hdr.tlen = htons(IP_HDR_SIZE + size);
  pkt.ip_hdr.id = htons(id++);
  pkt.ip_hdr.flags_offset = htons(0x4000);
  pkt.ip_hdr.ttl = INITIAL_TTL;
  pkt.ip_hdr.protocol = protocol;
  pkt.ip_hdr.src_ip = htonl(src_ip);
  pkt.ip_hdr.dst_ip = htonl(dst_ip);
  pkt.ip_hdr.checksum = 0;
  pkt.ip_hdr.checksum = checksum(&pkt.ip_hdr, IP_HDR_SIZE);
  memmove(pkt.transport_payload, buffer, size);

  if (dst_ip == 0xffffffff) {
    uchar destMAC[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    ether_send(destMAC, 0x0800, &pkt, IP_HDR_SIZE + size);
  }
  else {
    uchar *destMAC = arp_resolve(dst_ip);
    if(destMAC == 0)
      return;
    /* for(int i = 0; i < 6; i++) {
      cprintf("%x ", destMAC[i]);
    }
    cprintf("\n");*/
    ether_send(destMAC, 0x0800, &pkt, IP_HDR_SIZE + size);
  }
}
