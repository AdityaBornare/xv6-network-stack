#include "types.h"
#include "user.h"

int main() {
  int fd = socket(SOCK_STREAM);
  printf(1, "fd = %d\n", fd);
  exit();
}
