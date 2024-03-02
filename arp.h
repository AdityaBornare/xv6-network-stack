typedef struct arp_header {
    ushort hwd_type;
    ushort protocol;
    ushort hwd_length;
    ushort pro_length;
    ushort op_req;
} arp_header;

typedef struct arp_entry {
    arp_header arp_header; 
    ip_header ip_address;   
    unsigned char mac_address[6]; 
} arp_entry;