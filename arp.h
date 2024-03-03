#define ARP_HTYPE_ETHERNET 1
#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2
#define HADDR_SIZE 6

struct arp_packet {
  ushort hwd_type;
  ushort prot_type;
  uchar hwd_length;
  uchar prot_length;
  ushort op;
  uchar sender_haddr[HADDR_SIZE];
  uint sender_paddr;
  uchar target_haddr[HADDR_SIZE];
  uint target_paddr;
} __attribute__ ((packed));

typedef struct arp_packet arp_packet;

typedef struct arp_entry {
  uint ip;
  uchar mac[HADDR_SIZE];
  uint valid_until;
  uint is_valid;
} arp_entry;
