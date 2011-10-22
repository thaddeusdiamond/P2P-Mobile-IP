// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is the implementation for our sample echo app

#include <iostream>
#include "Applications/EchoApp.h"

void EchoApp::Run() {
  assert(CreateMobileNodeDelegate() == 0);
  listener_socket_ = CreateSocket(true);

  Signal::HandleSignalInterrupts();
  do {
    PrintReceivedData();
    EchoMessage(keyword_);
    sleep(3);
  } while (Signal::ShouldContinue());

  ShutDown("Normal termination.");
}

void EchoApp::ShutDown(const char* format, ...) {
  fprintf(stdout, "Shutting Down Echo App... ");

  va_list arguments;
  va_start(arguments, format);

  close(listener_socket_);

  mobile_node_->ShutDown(false, format, arguments);
  delete mobile_node_;

  fprintf(stdout, "OK\n");
  exit(1);
}

int EchoApp::CreateSocket(bool listener) {
  int connection;
  if ((connection = socket(domain_, transmission_type_, protocol_)) < 0)
    ShutDown("Could not open socket.");

  if ((connection = socket(domain_, transmission_type_, protocol_)) < 0)
    ShutDown("Error creating socket");

  // Next, we formalize the socket's structure using standard C conventions
  struct sockaddr_in socket_in;
  memset(&socket_in, 0, sizeof(socket_in));
  socket_in.sin_family = domain_;
  socket_in.sin_addr.s_addr = mobile_node_->GetTunnelInterface();


  struct sockaddr_in addr;
  struct ifreq ifr;
  int s = socket(domain_, transmission_type_, protocol_);

  strncpy(ifr.ifr_name, "tun0\0", sizeof(ifr.ifr_name)-1);
  if (ioctl(s, SIOCGIFADDR, &ifr) < 0)
    ShutDown("Could not get interface address of tunnel");
  close(s);

  addr = *((struct sockaddr_in *) &ifr.ifr_addr);
  std::cout << ifr.ifr_name << " address at " << addr.sin_addr.s_addr << std::endl;
  std::cout << "Tunnel at address " << mobile_node_->GetTunnelInterface() << std::endl;
//  socket_in.sin_port = htons(listener_port_);

  // Set the socket to be reusable
  int on = 1;
  if (setsockopt(connection, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<char*>(&on), sizeof(on)) < 0)
    ShutDown("Could not make the socket reusable");

  // Next we bind the incoming socket and listen on the port
  if (bind(connection, (struct sockaddr*) &socket_in, sizeof(socket_in)))
    ShutDown("Error binding socket to listen for peer");

  // Set the socket to be non-blocking
  int opts;
  if ((opts = fcntl(connection, F_GETFL)) < 0)
    ShutDown("Error getting the socket options");
  if (fcntl(connection, F_SETFL, opts | O_NONBLOCK) < 0)
    ShutDown("Error setting the socket to nonblocking");

  // Listen if that argument was specified
  if (listener) {
    if (listen(connection, MAX_CONNECTIONS))
      ShutDown("Error listening on socket");
    fprintf(stdout, "Listening for data on %d\n", ntohs(socket_in.sin_port));
    sleep(5);
  }

  return connection;
}

void EchoApp::PrintReceivedData() {
  fd_set read_set;
  FD_ZERO(&read_set);
  FD_SET(listener_socket_, &read_set);

  struct timeval wait;
  wait.tv_sec = 1;
  wait.tv_usec = 0;
  int ready = select(listener_socket_ + 1, &read_set,
                     reinterpret_cast<fd_set*>(0), reinterpret_cast<fd_set*>(0),
                     &wait);
  if (ready < 0)
    ShutDown("Error reading from socket pool");

  if (ready && FD_ISSET(listener_socket_, &read_set)) {
    struct sockaddr_in peer;
    socklen_t address_length = sizeof(peer);
    int connection = accept(listener_socket_, (struct sockaddr*) &peer,
                            &address_length);
    if (connection < 0)
      ShutDown("Error accepting connection");

    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    if (read(connection, buffer, sizeof(buffer)) < 1) {
      close(connection);
      fprintf(stderr, "Trying to read on closed connection\n");
      return;
    }
    
    // TODO(Thad): Change this to actually get IP from tunnel
    // Get actual destination it came from
    int outgoing_ip =   ((static_cast<unsigned int>(buffer[0]) << 24) |
                         (static_cast<unsigned int>(buffer[1]) << 16) |
                         (static_cast<unsigned int>(buffer[2]) << 8) |
                          static_cast<unsigned int>(buffer[3]));
    int outgoing_port = ((static_cast<unsigned char>(buffer[4]) << 8) |
                          static_cast<unsigned char>(buffer[5]) << 8 >> 8);
    char* message = buffer + 6;
    
    // Figure out the reverse IP address
    char peer_ip_address[INET_ADDRSTRLEN];
    snprintf(peer_ip_address, sizeof(peer_ip_address), "%d.%d.%d.%d",
             outgoing_ip << 24 >> 24, outgoing_ip << 16 >> 24,
             outgoing_ip << 8 >> 24, outgoing_ip >> 24);
    
    // Echo back the peer's communication or bury our own
    if (trim(message) && strcmp(keyword_, message))
      EchoMessage(message, peer_ip_address, outgoing_port);
    else if (buffer[0])
      fprintf(stdout, "Received back our own communication. Burying...\n");

    close(connection);
  }
}

void EchoApp::EchoMessage(char* message, char* peer_ip_address, 
                          int peer_port) {
  if (!peer_ip_address)
    peer_ip_address = peer_ip_address_;
  if (!peer_port)
    peer_port = peer_port_;
    
  struct hostent* peer_entity;
  if (!(peer_entity = gethostbyname(peer_ip_address_)))
    ShutDown("Failed to get home agent at specified IP");

  struct sockaddr_in peer_in;
  memset(&peer_in, 0, sizeof(peer_in));
  peer_in.sin_family = domain_;
  peer_in.sin_addr.s_addr = ((struct in_addr *)(peer_entity->h_addr))->s_addr;
  peer_in.sin_port = htons(peer_port);

  int connection = CreateSocket();
  if (connect(connection, (struct sockaddr*) &peer_in, sizeof(peer_in)))
    ShutDown("Failed to register socket with peer (%d:%d)",
        ((struct in_addr *)(peer_entity->h_addr))->s_addr, data_port_);
  
  if (write(connection, message, strlen(message)) < 0)
    ShutDown("Could not write to the peer");

  if (strcmp(message, keyword_))
    fprintf(stdout, "Received a message, \"%s\" from our friend!\n", message);

  fprintf(stdout, "Sending %s to our friend\n", message);
  close(connection);
}

int EchoApp::CreateMobileNodeDelegate() {
  pthread_t mobile_node_daemon;
  mobile_node_ =
    new SimpleMobileNode(home_ip_address_, home_port_, change_port_,
                         data_port_, listener_port_);

  int thread_status = pthread_create(&mobile_node_daemon, NULL,
                                     &RunMobileAgentThread, mobile_node_);
  sleep(5);

  return thread_status;
}
