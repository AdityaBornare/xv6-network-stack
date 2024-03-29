enum socket_type {TCP};

struct socket {
  uint addr;
  ushort src_port;
  socket_type type;
  struct tcp_connection tcon;
};
