#define MAX_WAITING_CLIENTS 10

// socket states
enum socket_states {
  SOCKET_FREE,
  SOCKET_UNBOUND,
  SOCKET_BOUND,
  SOCKET_WAIT,
  SOCKET_CONNECTED,
};

struct socket {
  uint addr;
  ushort port;
  enum { TCP } type;
  struct queue waitqueue;
  char state;
  char *buffer;
  int offset;
  int end;
  struct tcp_connection tcon;
};

struct port {
  int pid;
  struct socket *active_socket;
  struct socket *passive_socket;
};

