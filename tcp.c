#include "types.h"
#include "defs.h"
#include "queue.h"
#include "tcp.h"
#include "socket.h"
#include "ip.h"

extern struct port ports[];

struct tcp_request requests[MAX_PENDING_REQUESTS];

void tcp_receive(void *tcp_segment, int size, uint dst_ip) {
  struct tcp_packet *rx_pkt = (struct tcp_packet*) tcp_segment;
  ushort port = htons(rx_pkt->header.dst_port);
  uchar flags = rx_pkt->header.flags;
  struct socket *soc;
  int i;

  if(ports[port].pid == -1)
    return;
  if((soc = ports[port].socket) == 0)
    return;
  if(soc->state == SOCKET_UNBOUND)
    return;

  uint pseudo_ip_sum = 0;
  pseudo_ip_sum += (MYIP & 0xffff) + ((MYIP >> 16) & 0xffff);
  pseudo_ip_sum += (dst_ip & 0xffff) + ((dst_ip >> 16) & 0xffff);
  pseudo_ip_sum += htons((ushort)IP_PROTOCOL_TCP);
  pseudo_ip_sum += htons(size);

  if(checksum(rx_pkt, (rx_pkt->header.offset >> 4) << 2 , pseudo_ip_sum) != 0)
    return;

  if((flags & TCP_FLAG_SYN) != 0) {
    if(soc->tcon.state == TCP_LISTEN) {
      for(i = 0; i < MAX_PENDING_REQUESTS; i++) {
        if(requests[i].client_ip != 0) 
          break;
      }
      requests[i].client_ip = dst_ip;
      memmove(&requests[i].request_packet, rx_pkt, size);
      enqueue(&soc->waitqueue, (int) &requests[i]);

      if(soc->state == SOCKET_WAIT)
      wakeup(&ports[port]);
    }

    else if(soc->tcon.state == TCP_SYN_SENT && (flags & TCP_FLAG_ACK) != 0) {
      if(soc->tcon.dst_addr != dst_ip || soc->tcon.dst_port != htons(rx_pkt->header.src_port))
        return;
      memmove(soc->buffer, rx_pkt, size);
      wakeup(&ports[port]);
    }
    return;
  }

  if(soc->tcon.state == TCP_SYNACK_SENT && (flags & TCP_FLAG_ACK) != 0) {
    if(soc->tcon.dst_addr != dst_ip || soc->tcon.dst_port != htons(rx_pkt->header.src_port))
        return;
    memmove(soc->buffer, rx_pkt, size);
    wakeup(&ports[port]);
    return;
  }
}

void tcp_send(ushort src_port, ushort dst_port, uint dst_ip, uint seq_num, uint ack_num, uchar flags, uchar hdr_size, void* data, int data_size) {
  // Construct TCP packet
  struct tcp_packet packet;
  uint total_size = hdr_size + data_size;
  packet.header.src_port = htons(src_port);
  packet.header.dst_port = htons(dst_port);
  packet.header.seq_num = htonl(seq_num);
  packet.header.ack_num = htonl(ack_num);
  packet.header.offset = (hdr_size >> 2) << 4;
  packet.header.flags = flags;
  packet.header.window_size = htons(WINDOW_SIZE);
  packet.header.checksum = 0;
  packet.header.urgent_ptr = 0;

  // Copy data into packet
  memmove(packet.options_data, data, data_size);

  // Calculate TCP header checksum
  int pseudo_ip_sum = 0;
  pseudo_ip_sum += (MYIP & 0xffff) + ((MYIP >> 16) & 0xffff);
  pseudo_ip_sum += (dst_ip & 0xffff) + ((dst_ip >> 16) & 0xffff);
  pseudo_ip_sum += htons((ushort)IP_PROTOCOL_TCP);
  pseudo_ip_sum += htons((ushort)total_size);

  packet.header.checksum = checksum(&packet.header, hdr_size, pseudo_ip_sum);

  // Send packet using IP layer
  ip_send(IP_PROTOCOL_TCP, &packet, MYIP, dst_ip, total_size);

  return;
}
