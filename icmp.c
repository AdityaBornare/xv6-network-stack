#include "types.h"
#include "defs.h"
#include "ip.h"
#include "icmp.h"

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
  icmp_header->sum = checksum((ushort*)icmp_header, size, 0);

  // Send ICMP Echo Reply packet using IP layer
  ip_send(IP_PROTOCOL_ICMP, buffer, MYIP, source_ip, size);
}
