typedef struct {
          unsigned char          dst[6];
          unsigned char          src[6];
          unsigned short          ethertype;
          unsigned char          data[1500];
} ether_pack;

void send_ether_packet(unsigned char* destMAC,unsigned char* srcMAC,unsigned short ethertype,unsigned char* payload);
