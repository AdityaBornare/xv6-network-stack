#define MAX_TRANSPORT_PAYLOAD_SIZE 1480 //considering Maximum Transmission Unit (MTU) of ethernet is 1500 bytes
#define IP_ADDR_SIZE 4

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
  ushort offset;                  // Flags (3 bits) and Fragment Offset (13 bits)
  uchar ttl;                      // Time to live (TTL)
  uchar protocol;                 // Protocol used in the next layer (TCP/UDP)
  ushort checksum;                // Header checksum
  uchar src_ip[IP_ADDR_SIZE];     // Source IP address
  uchar dst_ip[IP_ADDR_SIZE];     // Destination IP address
}ip_header;

typedef struct ip_packet {
    ip_header ip_hdr;
    uchar transport_payload[MAX_TRANSPORT_PAYLOAD_SIZE];
} ip_packet;
