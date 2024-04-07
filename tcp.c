#include "types.h"
#include "defs.h"
#include "stddef.h"
#include "tcp.h"
#include "ip.h"

void tcp_receive() {
  // Extract TCP segment from the IP packet
  // Additional processing based on TCP header information
  // Implement logic to handle different TCP flags (SYN, ACK, etc.)
}

void tcp_send(ushort src_port, ushort dst_port, uint dst_ip, uint seq_num, uint ack_num, uchar flags, ushort window_size, void* data, int data_size) {
  // Construct TCP packet
  struct tcp_packet packet;
  int total_size = sizeof(struct tcp_hdr) + data_size;
  packet.header.src_port = htons(src_port);
  packet.header.dst_port = htons(dst_port);
  packet.header.seq_num = htonl(seq_num);
  packet.header.ack_num = htonl(ack_num);
  packet.header.offset = (sizeof(struct tcp_hdr) >> 2) << 4;
  packet.header.flags = flags;
  packet.header.window_size = htons(window_size);

  // Copy data into packet
  memmove(packet.data, data, data_size);

  // Calculate TCP header checksum
  packet.header.checksum = 0; // Placeholder for checksum calculation

  // Send packet using IP layer
  ip_send(IP_PROTOCOL_TCP, &packet, MY_IP, dst_ip, total_size);

  return;
}
