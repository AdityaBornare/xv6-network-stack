#define MAX_WAITING_CLIENTS 10

// socket states
enum socket_states {
  SOCKET_FREE,
  SOCKET_UNBOUND,
  SOCKET_BOUND,
  SOCKET_LISTENING,
  SOCKET_ACCEPTING,
  SOCKET_CONNECTING,
  SOCKET_WAITING_FOR_ACK,
  SOCKET_CONNECTED
};

struct socket {
  uint addr;
  ushort port;
  enum { TCP } type;
  struct queue waitqueue;
  char state;
  char *buffer;
  struct tcp_connection tcon;
};

struct port {
  int pid;
  struct socket *socket;
};

