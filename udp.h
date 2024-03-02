#define UDP_HEADER_LENGTH 8
#define MAX_APP_PLAYLOAD_SIZE 1472

typedef struct udp_header {
  ushort src_port;
  ushort dst_port;
  ushort length;
  ushort checksum;
} udp_header;

typedef struct udp_packet {
  udp_header udp_hdr;
  uchar app_playload[MAX_APP_PLAYLOAD_SIZE];
} udp_packet;
