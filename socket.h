#define MAX_WAITING_CLIENTS 10

enum StatusBits {
  SOCKET_BOUND     = 0x01,
  SOCKET_LISTENING = 0x02,
  SOCKET_CONNECTED = 0x04,
};

struct socket {
  uint addr;
  ushort port;
  enum { TCP } type;
  char status;
  char waitqueue[MAX_WAITING_CLIENTS];
  char buffer[MSS];
  struct tcp_connection tcon;
};
