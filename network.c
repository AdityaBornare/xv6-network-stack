#include "types.h"
#include "defs.h"
#include "network.h"

void network_init(){

  // Initialize network layer, setup routing tables, etc.

};

void network_receive(void* ip_dgram, int dsize){
  
  // Extract the network layer packet from the Ethernet frame

  // Process the received packet
  // Perform routing, packet validation, and other network layer tasks

  // Pass the transport payload to the transport layer (UDP/TCP)
  // The transport layer will handle the headers internally

};

void network_send(uchar tos, uchar ttl, ushort id, uchar protocol, uchar* buffer, uchar* src_ip, uchar* dst_ip, int size){
  
  // Perform any necessary processing before sending
  // (e.g., updating IP headers, routing information)

  // Encapsulate the packet in an Ethernet frame
  // Copy the network layer packet into the frame payload
  // Pass the frame to the data link layer for transmission

};

