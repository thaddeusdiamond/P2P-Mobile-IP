// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an implementation for a home agent as specified by the Mobile IP
// protocol

#include "HomeAgent/HomeAgent.h"
#include <iostream>

#define die(MSG) { perror(MSG); exit(1); }

// TODO(Thad): How do we remove mobile agents?

void HomeAgent::Run() {
  // Create an iterator for later use
  map<int, int>::iterator it;

  // Create a socket connection to listen on
  int listening_socket = CreateSocket(listener_port_);
  fd_limit_ = listening_socket + 1;

  do {
    // Set the various file descriptors
    fd_set read_set;
    FD_ZERO(&read_set);
    
    // Set the various read and write sets for select
    FD_SET(listening_socket, &read_set);
    for (it = connections_in_.begin(); it != connections_in_.end(); it++)
      FD_SET((*it).second, &read_set);
    for (it = connections_out_.begin(); it != connections_out_.end(); it++)
      FD_SET((*it).second, &read_set);

    // Set the wait time struct appropriately
    struct timeval wait;
    wait.tv_sec = 1;
    wait.tv_usec = 0;

    // Pull the list of ready connections
    int ready = select(fd_limit_, &read_set, (fd_set*) 0, (fd_set*) 0, &wait);
    if (ready < 0)
      die("Error reading from socket pool");
  
    // Data available
    if (ready) {
      for (it = connections_in_.begin(); it != connections_in_.end(); it++) {
        if (FD_ISSET((*it).second, &read_set))
          ForwardPackets((*it).second);
      }
      
      for (it = connections_out_.begin(); it != connections_out_.end(); it++) {
        if (FD_ISSET((*it).second, &read_set))
          RelabelPackets((*it).second);
      }
      
      if (FD_ISSET(listening_socket, &read_set))
        AddMobileAgent(next_available_port_++, listening_socket);
    }

  } while (true);
}

int HomeAgent::CreateSocket(unsigned short port) const {
  // First, we open up a listening socket
  int new_socket;
  if ((new_socket = socket(AF_INET, transmission_type_, 0)) < 0)
    die("Error creating socket");

  // Next, we formalize the socket's structure using standard C conventions
  struct sockaddr_in socket_in;
  memset(&socket_in, 0, sizeof(socket_in));
  socket_in.sin_family = AF_INET;
  socket_in.sin_addr.s_addr = INADDR_ANY;
  socket_in.sin_port = htons(port);

  // Next we bind the incoming socket and listen on the port
  if (bind(new_socket, (struct sockaddr *) &socket_in, sizeof(socket_in)))
    die("Error binding socket");
  if (listen(new_socket, MAX_CONNECTIONS))
    die("Error listening on socket");

  // Set the socket to be non-blocking
  int opts;
  if ((opts = fcntl(new_socket, F_GETFL)) < 0)
    die("Error getting the socket options");
  if (fcntl(new_socket, F_SETFL, opts | O_NONBLOCK) < 0)
    die("Error setting the socket to nonblocking");

  return new_socket;
}

int HomeAgent::AddMobileAgent(unsigned short port, int socket) {
  if (connections_in_.size() > MAX_CONNECTIONS)
    return false;
    
  // Accept the tunnel
  int tunnel = accept(socket, NULL, NULL);
  if (tunnel < 0)
    die("Error accepting connection");
    
  // Set the socket to be non-blocking
  int opts;
  if ((opts = fcntl(tunnel, F_GETFL)) < 0)
    die("Error getting the socket options");
  if (fcntl(tunnel, F_SETFL, opts | O_NONBLOCK) < 0)
    die("Error setting the socket to nonblocking");
  
  // Open on a new port so that incoming traffic can be directed
  int outbound = CreateSocket(port);

  // Update the socket map, incoming connections and highest file descriptor
  connections_in_[outbound] = tunnel;
  connections_out_[tunnel] = outbound;
  fd_limit_ = outbound + 1;
  
  std::cout << "Successfully connected at socket #" << tunnel << ", with " <<
    "outgoing socket #" << outbound << " (port #" << port << ")" <<
    std::endl;
  
  return socket;
}

bool HomeAgent::ForwardPackets(int tunnel) {
  // int socket = connections_[recipient];
  // TODO(Thad): Send packets along that port
  
  
  std::cout << "Forwarding Packets along the tunnel on socket #" << tunnel <<
    std::endl;

  return false;
}

bool HomeAgent::RelabelPackets(int outbound) const {
  // TODO(Thad): Relabel the outgoing packets with our name
  
  std::cout << "Relabeling outgoing packets for socket #" << outbound <<
    std::endl;
  
  return false;
}

bool HomeAgent::RemoveMobileAgent(int tunnel) {
  if (!connections_out_[tunnel])
    return false;

  close(connections_in_[connections_out_[tunnel]]);
  close(connections_out_[tunnel]);
  
  connections_in_.erase(connections_out_[tunnel]);
  connections_out_.erase(tunnel);
  
  return true;
}
