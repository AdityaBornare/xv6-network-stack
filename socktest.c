#include "types.h"
#include "user.h"

int main() {
  int fd = socket(SOCK_STREAM);
  int x = bind(fd, inet_addr("192.168.2.2"), 512);
  printf(1, "fd = %d\n", fd);
  printf(1, "bind = %d\n", x);
  exit();
}
