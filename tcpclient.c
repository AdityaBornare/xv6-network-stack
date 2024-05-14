#include "types.h"
#include "user.h"

int main() {
  int fd = socket(TCP);
  int c = connect(fd, inet_addr("192.168.2.2"), 512);
  printf(1, "%d\n", c);
  char buf[] = "hello";
  printf(1, "I am writing...\n");
  write(fd, buf, 5);
  printf(1, "I wrote %s\n", buf);
  close(fd); 
  exit();
}
