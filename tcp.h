#include "types.h"
#include "defs.h"

// TCP flags
#define TCP_FLAG_FIN  0x01  
#define TCP_FLAG_SYN  0x02
#define TCP_FLAG_RST  0x04
#define TCP_FLAG_PUSH 0x08
#define TCP_FLAG_ACK  0x10
#define TCP_FLAG_URG  0x20

// TCP header
struct tcp_hdr {
  ushort src_port;      // Source port number
  ushort dst_port;      // Destination port number
  uint seq_num;         // Sequence number
  uint ack_num;         // Acknowledgment number
  uchar offset;         // Data offset = header length
  uchar flags;          // Control flags
  ushort window_size;   // Window size
  ushort checksum;      // Header checksum
  ushort urgent_ptr;    // Urgent pointer
};

// TCP packet structure (header + data)
struct tcp_packet {
  tcp_hdr header;        // TCP header
  uchar data[];          // Data field (variable length)
};

// TCP connection
struct tcp_connection {
  uint dst_addr;
  ushort dst_port;
  uint seq_sent;
  uint ack_sent;
  uint seq_received;
  uint ack_received;
};
