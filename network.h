#ifndef _NETWORK_H
#define _NETWORK_H

#define MAX_TRANSPORT_PAYLOAD_SIZE 1480 //considering Maximum Transmission Unit (MTU) of ethernet is 1500 bytes
#define IP_ADDR_SIZE 4

/*note- ip header structure is according to rfc791-INTERNET PROTOCOL
  Not included the options field in header as it is needed for advanced or specialized networking feature
  can be considered in future if needed
  current header size is 20bytes i.e IHL(Internet Header Length) is 5
*/


typedef struct ip_header {
  unsigned char version_ihl;              // 4 bits for version, 4 bits for Internet Header Length (IHL)
  unsigned char tos;                      // Type of Service
  unsigned short tlen;                    // Total length of the IP packet (header + data)
  unsigned short id;                      // Identification for fragmented packets
  unsigned short offset;                  // Flags (3 bits) and Fragment Offset (13 bits)
  unsigned char ttl;                      // Time to live (TTL)
  unsigned char protocol;                 // Protocol used in the next layer (TCP/UDP)
  unsigned short checksum;                // Header checksum
  unsigned char src_ip[IP_ADDR_SIZE];     // Source IP address
  unsigned char dst_ip[IP_ADDR_SIZE];     // Destination IP address
}ip_header;

typedef struct ip_packet {
    ip_header ip_hdr;
    unsigned char transport_payload[MAX_TRANSPORT_PAYLOAD_SIZE];
}ip_packet;

#endif // _NETWORK_H

