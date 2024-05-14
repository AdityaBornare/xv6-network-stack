#include "types.h"
#include "user.h"

#define BUF_SIZE 100
#define NUM_REQUESTS 10

int main(int argc, char *argv[]) {
  if(argc < 3) {
    printf(1, "too few arguments\n");
    exit();
  }
  if(argc > 3) {
    printf(1, "too many arguments\n");
    exit();
  }

  int fd, a, n;
  char buf[BUF_SIZE];

  if((fd = socket(TCP)) == -1)
    exit();
  if(bind(fd, inet_addr(argv[1]), atoi(argv[2])) != 0)
    exit();
  if(listen(fd) != 0)
    exit();

  n = NUM_REQUESTS;
  printf(1, "server listening...\n");

  while(n--) {
    if((a = accept(fd)) == -1)
      exit();
    printf(1, "connected with client\n");

    int bytes = read(a, buf, BUF_SIZE - 1);
    buf[bytes] = 0;

    printf(1, "bytes read %d\n", bytes);
    printf(1, "message received: %s\n", buf);
    printf(1, "echoing the message...\n");
    write(a, buf, bytes);
    printf(1, "connection terminated\n\n");
    close(a);

    if(strcmp(buf, "exit") == 0) {
      close(fd);
      printf(1, "\nserver stopped\n");
      exit();
    }
  }

  close(fd);
  exit();
}
