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
  struct socket *psoc, *asoc;
  int i;

  uint pseudo_ip_sum = 0;
  pseudo_ip_sum += (MYIP & 0xffff) + ((MYIP >> 16) & 0xffff);
  pseudo_ip_sum += (dst_ip & 0xffff) + ((dst_ip >> 16) & 0xffff);
  pseudo_ip_sum += htons((ushort)IP_PROTOCOL_TCP);
  pseudo_ip_sum += htons(size);

  if(checksum(rx_pkt, size , pseudo_ip_sum) != 0)
    return;

  psoc = ports[port].passive_socket;
  asoc = ports[port].active_socket;

  if(ports[port].pid == -1)
    return;
  if(psoc == 0 && asoc == 0)
    return;

  if(psoc != 0) {
    if(psoc->tcon.state == TCP_LISTEN && (flags & TCP_FLAG_SYN) != 0 && !isqueuefull(psoc->waitqueue)) {
      for(i = 0; i < MAX_PENDING_REQUESTS; i++) {
        if(requests[i].client_ip != 0) 
          break;
      }
      requests[i].client_ip = dst_ip;
      memmove(&requests[i].request_packet, rx_pkt, size);
      enqueue(&psoc->waitqueue, (int) &requests[i]);
      if(psoc->state == SOCKET_WAIT)
        wakeup(&ports[port]);
      return;
    }

    if(psoc->tcon.state == TCP_SYNACK_SENT && (flags & TCP_FLAG_ACK) != 0) {
      if(psoc->tcon.dst_addr != dst_ip || psoc->tcon.dst_port != htons(rx_pkt->header.src_port))
        return;
      memmove(psoc->buffer, rx_pkt, size);
      wakeup(&ports[port]);
      return;
    }
  }

  if(asoc != 0) {
    if(asoc->tcon.dst_addr != dst_ip || asoc->tcon.dst_port != htons(rx_pkt->header.src_port))
      return;

    if(asoc->tcon.state == TCP_SYN_SENT && (flags & TCP_FLAG_ACK) != 0) {
      memmove(asoc->buffer, rx_pkt, size);
      wakeup(&ports[port]);
      return;
    }

    if(asoc->tcon.state == TCP_ESTABLISHED) {
      if((flags & TCP_FLAG_ACK) != 0) {
        uint ack = htonl(rx_pkt->header.ack_num);
        if(ack < asoc->tcon.base_seq || ack > asoc->tcon.next_seq) {
          return;
        }
        struct queued_packet *qp = (struct queued_packet *) getfront(asoc->tcon.window);
        while(!isqueueempty(asoc->tcon.window) && ack >= (qp->seq + qp->size - TCP_HEADER_MIN_SIZE)) {
          dequeue(&asoc->tcon.window);
          asoc->tcon.base_seq += qp->size;
          qp = (struct queued_packet *) getfront(asoc->tcon.window);
        }
        return;
      }

      int rx_hdr_size = rx_pkt->header.offset >> 2;

      if(asoc->tcon.ack_sent == htonl(rx_pkt->header.seq_num)) {
        memmove(asoc->buffer + asoc->end, ((void*) rx_pkt) + rx_hdr_size, size - rx_hdr_size);
        asoc->end += size - rx_hdr_size;
        if(asoc->state == SOCKET_WAIT)
          wakeup(&ports[port]);
        asoc->tcon.seq_received = htonl(rx_pkt->header.seq_num);
        uint ack = asoc->tcon.seq_received + (size - rx_hdr_size);
        tcp_send_ack(asoc, ack);
        asoc->tcon.ack_sent = ack;
      }
      else {
        tcp_send_ack(asoc, asoc->tcon.ack_sent);
      }
    }
    return;
  }
}

void tcp_pack(struct tcp_packet *packet, ushort src_port, ushort dst_port, uint dst_ip, uint seq_num, uint ack_num, uchar flags, uchar hdr_size, void* data, int data_size) {
  uint total_size = hdr_size + data_size;
  packet->header.src_port = htons(src_port);
  packet->header.dst_port = htons(dst_port);
  packet->header.seq_num = htonl(seq_num);
  packet->header.ack_num = htonl(ack_num);
  packet->header.offset = (hdr_size >> 2) << 4;
  packet->header.flags = flags;
  packet->header.window_size = htons(WINDOW_SIZE);
  packet->header.checksum = 0;
  packet->header.urgent_ptr = 0;

  // Copy data into packet
  memmove(packet->options_data, data, data_size);

  // Calculate TCP header checksum
  int pseudo_ip_sum = 0;
  pseudo_ip_sum += (MYIP & 0xffff) + ((MYIP >> 16) & 0xffff);
  pseudo_ip_sum += (dst_ip & 0xffff) + ((dst_ip >> 16) & 0xffff);
  pseudo_ip_sum += htons((ushort)IP_PROTOCOL_TCP);
  pseudo_ip_sum += htons((ushort)total_size);

  packet->header.checksum = checksum(&packet->header, total_size, pseudo_ip_sum);
  return;
}

void tcp_send(ushort src_port, ushort dst_port, uint dst_ip, uint seq_num, uint ack_num, uchar flags, uchar hdr_size, void* data, int data_size) {
  struct tcp_packet packet;
  
  // Construct TCP packet
  tcp_pack(&packet, src_port, dst_port, dst_ip, seq_num, ack_num, flags, hdr_size, data, data_size);

  uint total_size = hdr_size + data_size;

  // Send packet using IP layer
  ip_send(IP_PROTOCOL_TCP, &packet, MYIP, dst_ip, total_size);
  return;
}

void tcp_send_ack(struct socket* s, int ack) {
  tcp_send(
    s->port,
    s->tcon.dst_port,
    s->tcon.dst_addr,
    s->tcon.next_seq,
    ack, // acknowledgement to be sent
    TCP_FLAG_ACK,
    TCP_HEADER_MIN_SIZE,
    0,
    0
  );

  return;
}

static inline void tcp_resend(struct queued_packet *qp, uint dst_ip) {
  ip_send(IP_PROTOCOL_TCP, &qp->pkt, MYIP, dst_ip, qp->size);
}

void tcp_tx(struct socket *s, char *payload, int payload_size) {
  int i = 0;
  int window_offset = 0;
  int num_iter = payload_size / MSS + 1;

  while(i < num_iter) {
    if(!isqueuefull(s->tcon.window)) {
      int current_payload_size = MSS;
      if(i == payload_size/MSS)
        current_payload_size = payload_size % MSS;

      struct tcp_packet pkt;
      tcp_pack(
        &pkt,
        s->port,
        s->tcon.dst_port,
        s->tcon.dst_addr,
        s->tcon.next_seq,
        0,
        0,
        TCP_HEADER_MIN_SIZE,
        payload,
        current_payload_size
      );

      int total_size = TCP_HEADER_MIN_SIZE + current_payload_size;

      s->tcon.pkts[window_offset].size = total_size;
      s->tcon.pkts[window_offset].seq = s->tcon.next_seq;
      memmove(&s->tcon.pkts[window_offset].pkt, &pkt, total_size);
      enqueue(&s->tcon.window, (int) &s->tcon.pkts[window_offset]);
      window_offset = (window_offset + 1) % WINDOW_LENGTH;

      ip_send(IP_PROTOCOL_TCP, &pkt, MYIP, s->tcon.dst_addr, total_size);
      s->tcon.seq_sent = s->tcon.next_seq;
      s->tcon.next_seq += current_payload_size;
      i++;
    }
    if(isqueuefull(s->tcon.window)) {
      while(isqueuefull(s->tcon.window)) {
        delay(100);
        if(isqueuefull(s->tcon.window))
          break;
        tcp_resend((struct queued_packet *) getfront(s->tcon.window), s->tcon.dst_addr);
      }
    }

    if(i == num_iter) {
      while(s->tcon.base_seq != s->tcon.next_seq) {
        delay(100);
        if(s->tcon.base_seq != s->tcon.next_seq)
          break;
        tcp_resend((struct queued_packet *) getfront(s->tcon.window), s->tcon.dst_addr);
      }
    }
  }
}
