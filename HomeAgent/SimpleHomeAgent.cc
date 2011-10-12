// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an implementation for a home agent as specified by the Mobile IP
// protocol

#include "HomeAgent/SimpleHomeAgent.h"

#define die(MSG) { perror(MSG); exit(1); }

void SimpleHomeAgent::Run() {
  // Create an iterator for later use
  map<int, int>::iterator it;
  struct timeval wait;

  // Create a socket connection to listen on for new connections and changes
  int listening_socket = CreateSocket(listener_port_);
  int change_socket = CreateSocket(change_port_);
  int data_socket = CreateSocket(data_port_);
  fd_limit_ = data_socket + 1;

  do {
    // Set the various file descriptors
    fd_set read_set;
    FD_ZERO(&read_set);

    // Set the various read and write sets for select
    FD_SET(listening_socket, &read_set);
    FD_SET(change_socket, &read_set);
    FD_SET(data_socket, &read_set);
    for (it = connections_in_.begin(); it != connections_in_.end(); it++)
      FD_SET((*it).first, &read_set);

    // Set the wait time struct appropriately
    wait.tv_sec = 1;
    wait.tv_usec = 0;

    // Pull the list of ready connections
    int ready = select(fd_limit_, &read_set, reinterpret_cast<fd_set*>(0),
                       reinterpret_cast<fd_set*>(0), &wait);
    if (ready < 0)
      die("Error reading from socket pool");

    // Data available
    if (ready) {
      for (it = connections_in_.begin(); it != connections_in_.end(); it++) {
        if (FD_ISSET((*it).first, &read_set))
          ForwardPackets((*it).first);
      }

      if (FD_ISSET(data_socket, &read_set))
          RelabelPackets(data_socket);

      if (FD_ISSET(listening_socket, &read_set))
        AddMobileAgent(next_port_++, listening_socket);

      if (FD_ISSET(change_socket, &read_set))
        ChangeMobileAgent(change_socket);
    }
  } while (true);
}

int SimpleHomeAgent::CreateSocket(unsigned short port) const {
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

bool SimpleHomeAgent::AddMobileAgent(unsigned short out_port, int socket) {
  if (connections_in_.size() > MAX_CONNECTIONS)
    return false;

  // Accept the tunnel then close it out
  struct sockaddr_in peer;
  socklen_t address_length = sizeof(peer);
  int dummy = accept(socket, (struct sockaddr *) &peer, &address_length);
  if (dummy < 0)
    die("Error accepting connection");
  close(dummy);

  // Open on a new port so that incoming traffic can be directed
  int outbound = CreateSocket(out_port);

  // Update the socket map, incoming connections and highest file descriptor
  tunnel_identities_[peer.sin_addr.s_addr] = outbound;
  connections_in_[outbound] = peer.sin_addr.s_addr;
  fd_limit_ = outbound + 1;

  fprintf(stdout, "Successfully connected %s ", inet_ntoa(peer.sin_addr)),
  fprintf(stdout, "with outgoing socket #%d (port #%d)\n", outbound, out_port);

  return true;
}

bool SimpleHomeAgent::ChangeMobileAgent(int tunnel) {
  // Accept the connection detected on the tunnel
  struct sockaddr_in peer;
  socklen_t address_length = sizeof(peer);
  int connection = accept(tunnel, (struct sockaddr *) &peer, &address_length);
  if (connection < 0)
    die("Error accepting connection");

  // Write the data into memory (if it exists)
  char old_name[4096];
  memset(&old_name, 0, sizeof(old_name));

  // Read in the old name if it was given
  if (read(connection, old_name, sizeof(old_name)) < 0) {
    fprintf(stderr, "Trying to read on closed connection\n");
    close(connection);
    return false;
  }

  // Check for the user's identity and where to change it=
  int old_address = atoi(trim(old_name));
  if (tunnel_identities_.find(old_address) != tunnel_identities_.end()) {
    tunnel_identities_[peer.sin_addr.s_addr] = tunnel_identities_[old_address];
    tunnel_identities_.erase(old_address);
    fprintf(stdout, "Changed identity of %s to %s\n", old_name,
            inet_ntoa(peer.sin_addr));
  } else {
    fprintf(stderr, "User %s does not exist\n", old_name);
    close(connection);
    return false;
  }

  // Close the opened connection
  close(connection);
  return true;
}

bool SimpleHomeAgent::ForwardPackets(int outbound) {
  // Accept the connection detected on the tunnel
  struct sockaddr_in peer;
  socklen_t address_length = sizeof(peer);
  int connection = accept(outbound, (struct sockaddr *) &peer, &address_length);
  if (connection < 0)
    die("Error accepting connection");

  // Write the data into memory (if it exists)
  char buffer[4096];
  memset(&buffer, 0, sizeof(buffer));
  if (read(connection, buffer, sizeof(buffer)) < 0) {
    fprintf(stderr, "Trying to read on closed connection\n");
  } else {
    int forwarding_address = connections_in_[outbound];
    fprintf(stdout, "Received from outbound #%d: %s\n", outbound,
            buffer);
    fprintf(stdout, "Forwarding along the tunnel for IP %d\n",
            forwarding_address);

    // TODO(Thad): Get actual destination and then forward packets there along
    // proper outbound port
  }

  // Close the opened connection
  close(connection);
  return true;
}

bool SimpleHomeAgent::RelabelPackets(int mobile) {
  // Accept the connection detected on the outbound port
  struct sockaddr_in peer;
  socklen_t address_length = sizeof(peer);
  int connection = accept(mobile, (struct sockaddr *) &peer, &address_length);
  if (connection < 0)
    die("Error accepting connection");

  // Make sure we can find the user
  if (tunnel_identities_.find(peer.sin_addr.s_addr) == tunnel_identities_.end())
    return false;

  // Write the data into memory (if it exists)
  char buffer[4096];
  memset(&buffer, 0, sizeof(buffer));
  if (read(connection, buffer, sizeof(buffer)) < 0) {
    fprintf(stderr, "Trying to read on closed connection\n");
  } else {
    int outbound = tunnel_identities_[peer.sin_addr.s_addr];
    fprintf(stdout, "Received from IP %d: %s\n", peer.sin_addr.s_addr, buffer);
    fprintf(stdout, "Forwarding along the tunnel for IP %d\n", outbound);

    // TODO(Thad): Get actual destination and then forward packets there
  }

  // Close the opened connection
  close(connection);
  return true;
}

bool SimpleHomeAgent::RemoveMobileAgent(int mobile_ip) {
  if (tunnel_identities_.find(mobile_ip) == tunnel_identities_.end())
    return false;

  connections_in_.erase(tunnel_identities_[mobile_ip]);
  tunnel_identities_.erase(mobile_ip);
  close(tunnel_identities_[mobile_ip]);

  return true;
}
