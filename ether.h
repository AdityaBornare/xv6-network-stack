typedef struct {
          unsigned char          dst[6];
          unsigned char          src[6];
          unsigned short          ethertype;
          unsigned char          data[1500];
} ether_pack;
