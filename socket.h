struct port {
  uchar used;
  int pid;
}

struct socket {
  uint addr;
  ushort src_port;
  struct tcp_connection;
}
