#define DHCP_OPTION_TYPE 53;

#define DHCPDISCOVER 1
#define DHCPOFFER 2
#define DHCPREQUEST 3
#define DHCPDECLINE 4
#define DHCPACK 5
#define DHCPNAK 6
#define DHCPRELEASE 7
#define DHCPINFORM 8

struct dhcp_option_msgtype {
  char code;
  char len;
  char type;
};

typedef struct dhcp_msg {
  char op;                    // Operation code, 1 = REQUEST, 2 = REPLY
  char htype;                 // Hardware type, 1 = ethernet
  char hlen;                  // Hardware address length, 6 for mac address
  char hops;                  // Hop count, The number of times a message has been forwarded
  uint xid;                   // Transaction id, random number chosen by the client
  ushort secs;                // Currently reserved and set to 0
  ushort flags;               // bit 1 - broadcast flag, other bits are reserved and set to 0
  uint ciaddr;                // Client IP address, set to 0 if not known
  uint yiaddr;                // Your IP address, filled by server in reply message
  uint siaddr;                // Server IP address, filled by server in reply message
  uint giaddr;                // Gateway IP address, illed by the server in reply message
  char chaddr[16];            // Physical address of the client
  char sname[64];             // Server name, optionally filled by server
  char bootfile[128];         // Boot file name, optionally filled by server
  uint magic_cookie;          // fixed value
  struct dhcp_option_msgtype type;  // Option to convey the type of DHCP message
  char end_code;
} dhcp_msg;
