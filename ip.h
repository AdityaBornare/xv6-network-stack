#define MAX_TRANSPORT_PAYLOAD_SIZE 1480 //considering Maximum Transmission Unit (MTU) of ethernet is 1500 bytes
#define IP_ADDR_SIZE 4
#define MINIMUM_IHL 5
#define IP_HDR_SIZE 20
#define INITIAL_TTL 64
#define IP_PROTOCOL_ICMP 0x01
#define IP_PROTOCOL_TCP  0x06
#define IP_PROTOCOL_UDP  0x11

/*note- ip header structure is according to rfc791-INTERNET PROTOCOL
  Not included the options field in header as it is needed for advanced or specialized networking feature
  can be considered in future if needed
  current header size is 20bytes i.e IHL(Internet Header Length) is 5
*/

typedef struct ip_header {
  uchar version_ihl;              // 4 bits for version, 4 bits for Internet Header Length (IHL)
  uchar tos;                      // Type of Service
  ushort tlen;                    // Total length of the IP packet (header + data)
  ushort id;                      // Identification for fragmented packets
  ushort flags_offset;            // Flags (3 bits) and Fragment Offset (13 bits)
  uchar ttl;                      // Time to live (TTL)
  uchar protocol;                 // Protocol used in the next layer (TCP/UDP)
  ushort checksum;                // Header checksum
  uint src_ip;                    // Source IP address
  uint dst_ip;                    // Destination IP address
} ip_header;

typedef struct ip_packet {
    ip_header ip_hdr;
    uchar transport_payload[MAX_TRANSPORT_PAYLOAD_SIZE];
} ip_packet;
