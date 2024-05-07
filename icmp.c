#include "types.h"
#include "defs.h"
#include "ip.h"
#include "icmp.h"

#define ICMP_PACKET_SIZE 64
#define ICMP_DATA_SIZE (ICMP_PACKET_SIZE - ICMP_HDR_SIZE)
#define ICMP_PAYLOAD_PATTERN 'A'

void icmp_receive(void* icmp_pkt, int pkt_size, uint src_ip) {
  // Assuming icmp_pkt is a complete ICMP packet including the header
  icmp_hdr* icmp_header = (icmp_hdr*)icmp_pkt;

  // Add additional logic based on the ICMP type and code
  switch (icmp_header->type) {
    case ICMP_TYPE_ECHO:
      // Handle ICMP Echo Request
      icmp_send_echo_reply(icmp_pkt, pkt_size, src_ip);
      break;

    case ICMP_TYPE_ECHOREPLY:
      // Handle ICMP Echo Reply
      cprintf("Received ICMP Echo Reply\n");
      break;

    // Add cases for other ICMP types as required in future

    default:
      // Unknown ICMP type, handle or log accordingly
      cprintf("Unknown ICMP type: %d\n", icmp_header->type);
      break;
  }
}

void icmp_send_echo_reply(void* icmp_request_pkt, int pkt_size, uint source_ip) {
  // Allocate buffer for ICMP Echo Reply
  int size = pkt_size;
  uchar buffer[size];
  memmove(buffer, icmp_request_pkt, size);

  // Modify ICMP type to Echo Reply
  icmp_hdr* icmp_header = (icmp_hdr*)buffer;
  icmp_header->type = ICMP_TYPE_ECHOREPLY;

  // Recalculate ICMP checksum
  icmp_header->sum = 0;
  icmp_header->sum = checksum((ushort*)icmp_header, size);

  // Send ICMP Echo Reply packet using IP layer
  ip_send(IP_PROTOCOL_ICMP, buffer, MY_IP, source_ip, size);
}

void icmp_send_echo_request(uint dst_ip) {
  // Allocate buffer for ICMP Echo Request packet
  uchar buffer[ICMP_PACKET_SIZE];
  memset(buffer, 0, ICMP_PACKET_SIZE);

  icmp_hdr* icmp_header = (icmp_hdr*)buffer;
  icmp_header->type = ICMP_TYPE_ECHO;
  icmp_header->code = 0; // No code for Echo Request
  icmp_header->sum = 0; // Initialize checksum to 0 (to be calculated later)
  icmp_header->un.echo.id = htons(0); // ID field 
  icmp_header->un.echo.seq = htons(0); // Sequence number

  // Fill data section with payload pattern
  for (int i = ICMP_HDR_SIZE; i < ICMP_PACKET_SIZE; i++) {
      buffer[i] = ICMP_PAYLOAD_PATTERN;
  }

  // Calculate ICMP checksum
  icmp_header->sum = checksum((ushort*)icmp_header, ICMP_PACKET_SIZE);

  // Send ICMP Echo Request packet using IP layer
  ip_send(IP_PROTOCOL_ICMP, buffer, MY_IP, dst_ip, ICMP_PACKET_SIZE);
}

