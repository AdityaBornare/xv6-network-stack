#include "types.h"
#include "user.h"

int main() {
  int fd = socket(TCP);
  printf(1, "bind = %d\n", bind(fd, inet_addr("192.168.2.2"), 512));
  printf(1, "listen = %d\n", listen(fd));
  int a = accept(fd);
  printf(1, "accept = %d\n", a);
  char buf[10];
  int bytes = read(a, buf, 9);
  buf[bytes] = 0;
  printf(1, "I read %d bytes hehe\n", bytes);
  printf(1, "I read this: %s\n", buf);
  exit();
}
