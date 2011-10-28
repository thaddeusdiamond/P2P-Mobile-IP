// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an implementation for a home agent as specified by the Mobile IP
// protocol

#include "HomeAgent/SimpleHomeAgent.h"

void SimpleHomeAgent::Run() {
  // Create an iterator for later use
  map<int, list<int> >::iterator it;
  struct timeval wait;

  // Create a socket connection to listen on for new connections and changes
  listening_socket_ = CreateSocket(listener_port_);
  change_socket_ = CreateSocket(change_port_);
  data_socket_ = CreateSocket(data_port_);
  fd_limit_ = data_socket_ + 1;

  Signal::HandleSignalInterrupts();
  do {
    // Set the various file descriptors
    fd_set read_set;
    FD_ZERO(&read_set);

    // Set the various read and write sets for select
    FD_SET(listening_socket_, &read_set);
    FD_SET(change_socket_, &read_set);
    FD_SET(data_socket_, &read_set);
    for (it = connections_in_.begin(); it != connections_in_.end(); it++)
      FD_SET((*it).first, &read_set);

    // Set the wait time struct appropriately
    wait.tv_sec = 1;
    wait.tv_usec = 0;

    // Pull the list of ready connections
    int ready = select(fd_limit_, &read_set, reinterpret_cast<fd_set*>(0),
                       reinterpret_cast<fd_set*>(0), &wait);
    if (ready < 0)
      ShutDown("Error reading from socket pool");

    // Data available
    if (ready > 0) {
      for (it = connections_in_.begin(); it != connections_in_.end(); it++) {
        if (FD_ISSET((*it).first, &read_set))
          ForwardPackets((*it).first);
      }

      if (FD_ISSET(data_socket_, &read_set))
        RelabelPackets(data_socket_);

      if (FD_ISSET(listening_socket_, &read_set))
        AddMobileAgent(next_port_++, listening_socket_);

      if (FD_ISSET(change_socket_, &read_set))
        ChangeMobileAgent(change_socket_);
    }
  } while (Signal::ShouldContinue());

  ShutDown("Normal termination.");
}

void SimpleHomeAgent::ShutDown(const char* format, ...) {
  fprintf(stdout, "Shutting Down Home Agent... ");

  va_list arguments;
  va_start(arguments, format);
  fprintf(stderr, format, arguments);
  perror(" ");

  close(listening_socket_);
  close(change_socket_);
  close(data_socket_);

  map<int, list<int> >::iterator it;
  for (it = connections_in_.begin(); it != connections_in_.end(); it++)
    close((*it).first);

  fprintf(stdout, "OK\n");
  exit(1);
}

bool SimpleHomeAgent::AddMobileAgent(unsigned short out_port, int socket) {
  // Prevent overqueuing
  if (connections_in_.size() > MAX_CONNECTIONS)
    return false;

  // Accept the tunnel then close it out
  struct sockaddr_in peer;
  socklen_t address_length = sizeof(peer);
  int peer_connect = accept(socket, (struct sockaddr*) &peer, &address_length);
  if (peer_connect < 0)
    ShutDown("Error accepting connection");

  // Open on a new port so that incoming traffic can be directed
  int ip_address;
  int outbound = CreateSocket(out_port, true, true, &ip_address);

  // Update the socket map, incoming connections and highest file descriptor
  tunnel_identities_[peer.sin_addr.s_addr] = outbound;
  list<int> connection;
  connection.push_back(peer.sin_addr.s_addr);
  connection.push_back(peer.sin_port);
  connections_in_[outbound] = connection;
  fd_limit_ = outbound + 1;

  fprintf(stdout, "Successfully connected %s (%d:%d) ",
          inet_ntoa(peer.sin_addr), connections_in_[outbound].front(),
          ntohs(connections_in_[outbound].back())),
  fprintf(stdout, "with outgoing socket #%d (port #%d)\n", outbound, out_port);

  // Send back the information
  char buffer[31];
  snprintf(buffer, sizeof(buffer), "%-20d%-10d", ip_address, out_port);
  if (write(peer_connect, buffer, strlen(buffer)) < 0)
    ShutDown("Could not send back the IP Address and Port to peer.");

  // Close peer connection
  close(peer_connect);

  return true;
}

bool SimpleHomeAgent::RemoveMobileAgent(int mobile_ip) {
  if (tunnel_identities_.find(mobile_ip) == tunnel_identities_.end())
    return false;

  // Erase traces of the mobile IP from the data members and close outbound
  // socket
  close(tunnel_identities_[mobile_ip]);
  connections_in_.erase(tunnel_identities_[mobile_ip]);
  tunnel_identities_.erase(mobile_ip);

  return true;
}

bool SimpleHomeAgent::ChangeMobileAgent(int tunnel) {
  // Accept the connection detected on the tunnel
  struct sockaddr_in peer;
  socklen_t address_length = sizeof(peer);
  int connection = accept(tunnel, (struct sockaddr*) &peer, &address_length);
  if (connection < 0)
    ShutDown("Error accepting connection");

  // Write the data into memory (if it exists)
  char old_name[4096], old_port_name[4096];
  memset(old_name, 0, 4096);
  memset(old_port_name, 0, 4096);

  // Read in the old name if it was given
  if (read(connection, old_name, IP_NAME_LENGTH) < 0 ||
      read(connection, old_port_name, IP_PORT_LENGTH) < 0) {
    fprintf(stderr, "Trying to read on closed connection\n");
    close(connection);
    return false;
  }

  // Check for the user's identity and where to change it
  unsigned int old_address = atoi(trim(old_name));
  unsigned int new_address = peer.sin_addr.s_addr;
  unsigned int old_port = atoi(trim(old_port_name));
  unsigned int new_port = peer.sin_port;
  if (tunnel_identities_.find(old_address) != tunnel_identities_.end()) {
    // Don't change the address if it's still the same
    if (new_address != old_address || new_port != old_port) {
      list<int> connection;
      connection.push_back(peer.sin_addr.s_addr);
      connection.push_back(peer.sin_port);
      connections_in_[tunnel_identities_[old_address]] = connection;
      tunnel_identities_[new_address] = tunnel_identities_[old_address];
    }

    if (new_address != old_address)
      tunnel_identities_.erase(old_address);

    fprintf(stdout, "Changed identity of %s to %s (%d:%d)\n", old_name,
            inet_ntoa(peer.sin_addr), peer.sin_addr.s_addr, peer.sin_port);

  // User doesn't exist, close the connection
  } else {
    fprintf(stderr, "User %s does not exist\n", old_name);
    close(connection);
    return false;
  }

  // Close the opened connection
  close(connection);
  return true;
}

// TODO(Thad): This should be made persistent and is actually  all wrong, it
// needs to use a SOCK_RAW connection so that it can encapsulate those packets
// and merely forward them to the mobile node which is listening and then
// sends the decapsulated packets internally
bool SimpleHomeAgent::ForwardPackets(int outbound) {
  // Accept the connection detected on the tunnel
  struct sockaddr_in peer;
  socklen_t address_length = sizeof(peer);
  int connection = accept(outbound, (struct sockaddr*) &peer, &address_length);
  if (connection < 0)
    ShutDown("Error accepting connection");

  // Write the data into memory (if it exists)
  char buffer[4090];
  int length;
  memset(buffer, 0, sizeof(buffer));
  if ((length = read(connection, buffer, sizeof(buffer))) < 0) {
    fprintf(stderr, "Trying to read on closed connection\n");
  } else {
    int forwarding_address = connections_in_[outbound].front();
    int forwarding_port = connections_in_[outbound].back();
    fprintf(stdout, "Received from outbound #%d (%d:%d): %s\n", outbound,
            peer.sin_addr.s_addr, ntohs(peer.sin_port), trim(buffer));
    fprintf(stdout, "Forwarding along the tunnel for IP %d:%d\n",
            forwarding_address, ntohs(forwarding_port));

    // TODO(Thad): Remove
    // Find out where it came from
    unsigned char prefix[6];
    int peer_ip = peer.sin_addr.s_addr;
    prefix[0] = peer_ip >> 24;
    prefix[1] = peer_ip << 8 >> 24;
    prefix[2] = peer_ip << 16 >> 24;
    prefix[3] = peer_ip << 24 >> 24;

    unsigned short peer_port = ntohs(peer.sin_port);
    prefix[4] = peer_port >> 8;
    prefix[5] = peer_port << 8 >> 8;
    
    // Forward back to mobile agent
    struct sockaddr_in peer_in;
    memset(&peer_in, 0, sizeof(peer_in));
    peer_in.sin_family = domain_;
    peer_in.sin_addr.s_addr = forwarding_address;
    peer_in.sin_port = forwarding_port;

    int connection;
    if ((connection = socket(domain_, transmission_type_, protocol_)) < 0)
      ShutDown("Could not open a socket to mobile agent.");

    if (connect(connection, (struct sockaddr*) &peer_in, sizeof(peer_in)) < 0) {
      fprintf(stderr, "Could not connect to mobile agent.\n");
      return false;
    }

    char message[4096];
    memset(message, 0, sizeof(message));
    snprintf(message, strlen(buffer) + 7, "%c%c%c%c%c%c%s", prefix[0], 
             prefix[1], prefix[2], prefix[3], prefix[4], prefix[5], buffer);
    
    if (send(connection, message, strlen(buffer) + 6, 0) < 0) {
      fprintf(stderr, "Could not send data to mobile agent.\n");
      return false;
    }

    close(connection);
  }

  // Close the opened connection
  close(connection);
  return true;
}


// TODO(Thad): Instead of relabeling this should merely use a SOCK_RAW to
// send out the packets (because they already have the right header information)
// associated
bool SimpleHomeAgent::RelabelPackets(int mobile) {
  // TODO(Thad)
  // int raw_socket = socket(PACKET, UDP, UDP_PROTO);
  // write(raw_socket, buffer, sizeof(buffer));

  // Accept the connection detected on the outbound port
  struct sockaddr_in peer;
  socklen_t address_length = sizeof(peer);
  int connection = accept(mobile, (struct sockaddr*) &peer, &address_length);
  if (connection < 0)
    ShutDown("Error accepting connection");

  // Make sure we can find the user
  bool complete = true;
  int tunnel_id = peer.sin_addr.s_addr;
  if (tunnel_identities_.find(tunnel_id) != tunnel_identities_.end()) {
    char buffer[4096];
    memset(buffer, 0, 4096);

    // Read in the data from the socket
    if (read(connection, buffer, 4096) < 0) {
      fprintf(stderr, "Trying to read on closed connection\n");
    } else {
      int outbound = tunnel_identities_[tunnel_id];
      fprintf(stdout, "Received from IP %d: %s\n", tunnel_id, buffer + 6);

      // Get actual destination and then forward packets there
      int outgoing_ip =   ((static_cast<unsigned int>(buffer[0]) << 24) |
                           (static_cast<unsigned int>(buffer[1]) << 16) |
                           (static_cast<unsigned int>(buffer[2]) << 8) |
                            static_cast<unsigned int>(buffer[3]));
      int outgoing_port = ((static_cast<unsigned char>(buffer[4]) << 8) |
                            static_cast<unsigned char>(buffer[5]) << 8 >> 8);
      fprintf(stdout, "Forwarding along outbound socket #%d to %d:%d\n",
              outbound, outgoing_ip, outgoing_port);

      // Open connection and send packets
      struct sockaddr_in outbound_info;
      socklen_t outbound_info_length = sizeof(outbound_info);
      getsockname(outbound, (struct sockaddr*) &outbound_info, 
                  &outbound_info_length);
      
      struct sockaddr_in peer_in;
      memset(&peer_in, 0, sizeof(peer_in));
      peer_in.sin_family = domain_;
      peer_in.sin_addr.s_addr = outgoing_ip;
      peer_in.sin_port = htons(outgoing_port);
      
      close(outbound);
      outbound = CreateSocket(ntohs(outbound_info.sin_port), false, false);

      if (connect(outbound, (struct sockaddr*) &peer_in, sizeof(peer_in))) {
        fprintf(stderr, "Could not connect to outgoing connection on %d.\n",
                ntohs(outbound_info.sin_port));
        complete = false;
      }
      if (complete && send(outbound, buffer + 6, strlen(buffer + 6), 0) < 0) {
        fprintf(stderr, "There was an error sending to outbound peer.\n");
        complete = false;
      }
      
      close(outbound);
      tunnel_identities_[tunnel_id] = CreateSocket(
          ntohs(outbound_info.sin_port));
    }
  } else {
    fprintf(stderr, "Connection from IP %d refused\n", tunnel_id);
    complete = false;
  }

  // Close the opened connection
  close(connection);
  return complete;
}

int SimpleHomeAgent::CreateSocket(unsigned short port, bool should_listen, 
                                  bool nonblocking, int* ip_address) {
  // First, we open up a listening socket
  int new_socket;
  if ((new_socket = socket(domain_, transmission_type_, protocol_)) < 0)
    ShutDown("Error creating socket");
  
  // Next, we formalize the socket's structure using standard C conventions
  struct sockaddr_in socket_in;
  memset(&socket_in, 0, sizeof(socket_in));
  socket_in.sin_family = domain_;
  socket_in.sin_addr.s_addr = GetCurrentIPAddress();
  socket_in.sin_port = htons(port);

  // Set the socket to be non-blocking
  if (nonblocking) {
    int opts;
    if ((opts = fcntl(new_socket, F_GETFL)) < 0)
      ShutDown("Error getting the socket options");
    if (fcntl(new_socket, F_SETFL, opts | O_NONBLOCK) < 0)
      ShutDown("Error setting the socket to nonblocking");
  }
  
  // All sockets should be reusable to avoid TIME_WAIT
  int on = 1;
  if (setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<char*>(&on), sizeof(on)) < 0)
    ShutDown("Could not make the socket reusable");

  // Next we bind the incoming socket and listen on the port
  if (bind(new_socket, (struct sockaddr*) &socket_in, sizeof(socket_in)))
    ShutDown("Error binding socket");
  if (should_listen && listen(new_socket, MAX_CONNECTIONS)) {
    ShutDown("Error listening on socket");
  } else if (should_listen) {
    fprintf(stdout, "Now listening on port %d\n", port);
  }
  
  // Set the ip_address if the pointer was passed in
  if (ip_address) {
    struct sockaddr_in dummy;
    socklen_t dummy_size = sizeof(dummy);
    getsockname(new_socket, (struct sockaddr*) &dummy, &dummy_size);

    *ip_address = dummy.sin_addr.s_addr;
  }
  return new_socket;
}
