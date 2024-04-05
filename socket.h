struct socket {
  uint addr;
  ushort port;
  enum { TCP } type;
  struct tcp_connection tcon;
};
