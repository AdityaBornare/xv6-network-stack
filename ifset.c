#include "types.h"
#include "user.h"

int main(int argc, char **argv) {

  if(argc < 3) {
    printf(1, "Too few arguments.\n");
    exit();
  }

  ifset(inet_addr(argv[1]), inet_addr(argv[2]), inet_addr(argv[3]));
  exit();
}
