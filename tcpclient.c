#include "types.h"
#include "user.h"

int main() {
  int fd = socket(TCP);
  int c = connect(fd, inet_addr("192.168.2.2"), 512);
  printf(1, "%d\n", c);
  exit();
}
