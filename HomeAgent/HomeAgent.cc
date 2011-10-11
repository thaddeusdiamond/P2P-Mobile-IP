// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an implementation for a home agent as specified by the Mobile IP
// protocol

#include "HomeAgent/HomeAgent.h"

#define die(MSG) { perror(MSG); exit(1); }

void HomeAgent::Run() {
  if ((incoming_socket_ = socket(AF_INET, transmission_type_, 0)))
    die("Error creating socket");

  struct sockaddr_in socket_in;
  memset(&socket_in, 0, sizeof(socket_in));
  socket_in.sin_family = AF_INET;
  socket_in.sin_addr.s_addr = INADDR_ANY;
  socket_in.sin_port = htons(port_);

  if (bind(incoming_socket_, (struct sockaddr *) &socket_in, sizeof(socket_in)))
    die("Error binding socket");

  if (listen(incoming_socket_, 10))
    die("Error listening on socket");

  do {
  } while (true);
}

bool HomeAgent::ForwardPackets(IPAddress recipient, Packets data) {
  // int socket = connections_[recipient];
  // TODO(Thad): Send packets along that port

  return false;
}

bool HomeAgent::RelabelPackets(IPAddress recipient, Packets data) const {
  // TODO(Thad): Relabel the outgoing packets with our name
  return false;
}

bool HomeAgent::AddMobileAgent(IPAddress ip_address, unsigned short port) {
  if (connections_[ip_address])
    return false;

  // TODO(Thad): Create an actual socket for the user
  connections_[ip_address] = port;
  return true;
}

bool HomeAgent::RemoveMobileAgent(IPAddress ip_address) {
  if (!connections_[ip_address])
    return false;

  connections_.erase(ip_address);
  return true;
}
