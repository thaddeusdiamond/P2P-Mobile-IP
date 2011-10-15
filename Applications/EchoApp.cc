// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is the implementation for our sample echo app

#include "Applications/EchoApp.h"

void EchoApp::Run() {
  assert(CreateMobileNodeDelegate() == 0);
  int listener_socket = CreateListener();

  do {
    PrintReceivedData(listener_socket);
    EchoMessage();
    sleep(3);
  } while (true);
}

int EchoApp::CreateSocket() {
  int connection;
  if ((connection = socket(AF_INET, transmission_type_, 0)) < 0)
    die("Could not open socket.");

  return connection;
}

int EchoApp::CreateListener() {
  int listener_socket;
  if ((listener_socket = socket(AF_INET, transmission_type_, 0)) < 0)
    die("Error creating socket");

  // Next, we formalize the socket's structure using standard C conventions
  struct sockaddr_in socket_in;
  memset(&socket_in, 0, sizeof(socket_in));
  socket_in.sin_family = AF_INET;
  socket_in.sin_addr.s_addr = INADDR_ANY;
  socket_in.sin_port = htons(listener_port_);

  // Set the socket to be reusable
  int on = 1;
  if (setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<char*>(&on), sizeof(on)) < 0)
    die("Could not make the socket reusable");

  // Next we bind the incoming socket and listen on the port
  if (bind(listener_socket, (struct sockaddr *) &socket_in, sizeof(socket_in)))
    die("Error binding socket to listen for peer");
  if (listen(listener_socket, MAX_CONNECTIONS))
    die("Error listening on socket");

  // Set the socket to be non-blocking
  int opts;
  if ((opts = fcntl(listener_socket, F_GETFL)) < 0)
    die("Error getting the socket options");
  if (fcntl(listener_socket, F_SETFL, opts | O_NONBLOCK) < 0)
    die("Error setting the socket to nonblocking");

  fprintf(stdout, "Listening for data on %d\n", socket_in.sin_port);

  return listener_socket;
}

void EchoApp::PrintReceivedData(int listener_socket) {
  fd_set read_set;
  FD_ZERO(&read_set);
  FD_SET(listener_socket, &read_set);

  struct timeval wait;
  wait.tv_sec = 1;
  wait.tv_usec = 0;
  int ready = select(listener_socket + 1, &read_set,
                     reinterpret_cast<fd_set*>(0), reinterpret_cast<fd_set*>(0),
                     &wait);
  if (ready < 0)
    die("Error reading from socket pool");

  if (ready && FD_ISSET(listener_socket, &read_set)) {
    struct sockaddr_in peer;
    socklen_t address_length = sizeof(peer);
    int connection = accept(listener_socket, (struct sockaddr *) &peer,
                            &address_length);
    if (connection < 0)
      die("Error accepting connection");

    char buffer[4096];
    memset(&buffer, 0, sizeof(buffer));
    if (read(connection, buffer, sizeof(buffer)) < 0)
      fprintf(stderr, "Trying to read on closed connection\n");
    else
      fprintf(stdout, "Received %s\n", buffer);

    close(connection);
  }
}

void EchoApp::EchoMessage() {
  int connection = mobile_node_->RegisterSocket(CreateSocket(), 0,
                                                peer_ip_address_, peer_port_);
  if (send(connection, keyword_, strlen(keyword_), 0) < 0)
    die("Error sending a message to the peer.");

  fprintf(stdout, "Sending %s to our friend\n", keyword_);
  close(connection);
}

int EchoApp::CreateMobileNodeDelegate() {
  pthread_t mobile_node_daemon;
  IPADDRESS(home_ip) = "127.0.0.1";
  mobile_node_ =
    new SimpleMobileNode(home_ip, 16000, 16001, 16002, listener_port_);

  int thread_status = pthread_create(&mobile_node_daemon, NULL,
                                     &RunMobileAgentThread, mobile_node_);
  sleep(3);

  return thread_status;
}
