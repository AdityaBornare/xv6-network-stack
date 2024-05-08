#include "types.h"
#include "user.h"

#define TIMEOUT_MS 5000

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf(2, "Usage: %s <destination_ip>\n", argv[0]);
    exit();
  }

  // Send ICMP Echo Request to destination IP
  printf(1, "Sending ICMP Echo Request to %s\n", argv[1]);
  icmp_send_echo_request(inet_addr(argv[1]));

  // Wait for ICMP Echo Reply
  printf(1, "Waiting for ICMP Echo Reply...\n");
  int timeout = 0;
  while (!get_icmp_echo_reply_status()) {
    if (timeout >= TIMEOUT_MS) {
      printf(1, "Timeout: No ICMP Echo Reply received\n");
      exit();
    }
    sleep(1000); // Sleep for 1 millisecond
    timeout++;
  }

  printf(1, "ICMP Echo Reply received\n");
  exit();
}

