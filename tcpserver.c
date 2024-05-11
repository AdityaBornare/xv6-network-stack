#include "types.h"
#include "user.h"

int main() {
  int fd = socket(TCP);
  printf(1, "bind = %d\n", bind(fd, inet_addr("192.168.2.2"), 512));
  printf(1, "listen = %d\n", listen(fd));
  int a = accept(fd);
  printf(1, "accept = %d\n", a);
  exit();
}
