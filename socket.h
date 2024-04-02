struct socket {
  uint addr;
  ushort src_port;
  enum { TCP } type;
  struct tcp_connection tcon;
};
