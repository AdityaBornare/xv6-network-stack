#include "types.h"
#include "user.h"

int main() {
  int fd = socket(TCP);
  bind(fd, inet_addr("192.168.2.2"), 512);
  listen(fd);
  int a = accept(fd);
  printf(1, "%d\n", a);
  exit();
}
