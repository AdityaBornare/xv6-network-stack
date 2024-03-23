#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
  test(inet_addr(argv[1]));
  exit();
}
