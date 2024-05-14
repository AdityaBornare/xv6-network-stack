#include "types.h"
#include "user.h"

#define TIMEOUT_MS 50000
#define ICMP_PACKET_SIZE 64
#define MAX_PACKETS 10

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf(2, "Usage: %s <destination_ip>\n", argv[0]);
    exit();
  }
  
  // Send ICMP Echo Request to destination IP
  printf(1, "Sending PING Request to %s\n", argv[1]);
  ushort seq_no = 1;
  int pkt_no = 0;
  int pkt_responsed = 0;
  
  while(pkt_no<MAX_PACKETS){
    int start_ticks, end_ticks;
    icmp_send_echo_request(inet_addr(argv[1]),seq_no); 
    start_ticks = uptime();

    // Wait for ICMP Echo Reply
    //printf(1, "Waiting for ICMP Echo Reply...\n");
    int timeout = 0;
    int received = 1;
    while (!get_icmp_echo_reply_status()) {
      if (timeout >= TIMEOUT_MS) {
        printf(1, "Timeout: No ICMP Echo Reply received\n");
        received = 0;
        sleep(100);
        break;
      }
      timeout++;
    }
    
    if(received == 0){
      pkt_no++;
      continue;
    }
    else{
      pkt_responsed++;
    }
  
    end_ticks = uptime();
    seq_no++;
    int response_time_ms = ((end_ticks - start_ticks) * 10);  
    //printf(1, "PING Reply received\n"); 
    get_icmp_echo_reply_packet();
    printf(1, "time=%d ms\n", response_time_ms);
    pkt_no++;
    sleep(100);
  }
  int packet_loss = ((MAX_PACKETS - pkt_responsed)/MAX_PACKETS) * 100;
  printf(1, "--- %s ping statistics ---\n", argv[1]);
  printf(1, "%d packets transmitted, %d received, %d packet loss\n",pkt_no,pkt_responsed,packet_loss);
  exit();
}

