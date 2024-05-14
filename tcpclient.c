#include "types.h"
#include "user.h"

#define BUF_SIZE 100

int main(int argc, char *argv[]) {
  if(argc < 4) {
    printf(1, "too few arguments\n");
    exit();
  }
  if(argc > 4) {
    printf(1, "too many arguments\n");
    exit();
  }

  int fd;
  char buf[10];

  if((fd = socket(TCP)) == -1)
    exit();
  if(connect(fd, inet_addr(argv[1]), atoi(argv[2])) != 0)
    exit();

  write(fd, argv[3], strlen(argv[3]));
  printf(1, "sent message: %s\n", argv[3]);

  int bytes = read(fd, buf, BUF_SIZE - 1);
  buf[bytes] = 0;
  printf(1, "received message: %s\n", buf);
  close(fd); 
  exit();
}
