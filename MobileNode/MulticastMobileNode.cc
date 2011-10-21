// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is the implementation file for a multicasting mobile node.

#include "MobileNode/MulticastMobileNode.h"

int MulticastMobileNode::CreateTunnel(char* tunnel_name) {
  return -1;
//  application_sockets_[app_socket] = app_id;

//  struct hostent* home_entity;
//  if (!(home_entity = gethostbyname(home_ip_address_)))
//    die(NULL, "Failed to get home agent at specified IP");

//  struct sockaddr_in home_in;
//  memset(&home_in, 0, sizeof(home_in));
//  home_in.sin_family = domain_;
//  home_in.sin_addr.s_addr = ((struct in_addr *)(home_entity->h_addr))->s_addr;
//  home_in.sin_port = htons(data_port_);

//  // We handle SCTP specially by making a second socket
//  if (protocol == SCTP_PROTO) {
//    struct hostent* peer_entity;
//    if (!(peer_entity = gethostbyname(peer_ip_address)))
//      die(NULL, "Failed to get home agent at specified IP");

//    struct sockaddr_in peer_in;
//    memset(&peer_in, 0, sizeof(peer_in));
//    peer_in.sin_family = domain;
//    peer_in.sin_addr.s_addr = ((struct in_addr *)(peer_entity->h_addr))->s_addr;
//    peer_in.sin_port = htons(peer_port);

//    struct sockaddr_in addresses[2];
//    addresses[0] = home_in;
//    addresses[1] = peer_in;

//// TODO(Thad): Multicasting!!!
////   Label the association by which socket it is on and connect
////   sctp_assoc_t assoc_id = app_socket;
////   if (sctp_connectx(app_socket, (struct sockaddr*) addresses, 2, &assoc_id))
////     die(NULL, "Failed to connect using SCTP to home agent and peer.");

//  // Regular one-way connection to home
//  } else {
//    if (connect(app_socket, (struct sockaddr*) &home_in, sizeof(home_in)))
//      die(NULL, "Failed to register socket at the home agent");
//  }

//  return app_socket;
}
