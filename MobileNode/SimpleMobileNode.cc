// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is the implementation file for a simple mobile node.

#include "MobileNode/SimpleMobileNode.h"

void* SimpleMobileNode::Run() {
  assert(ConnectToHome(home_port_));
  do {
    sleep(5);
    assert(ChangeHomeIdentity());
  } while (true);
}

int SimpleMobileNode::RegisterSocket(int app_socket, int app_id,
                                     IPADDRESS(peer_ip_address),
                                     unsigned short peer_port) {
  application_sockets_[app_socket] = app_id;

// TODO(Thad): Re-routing is not quite right
//  struct hostent *host_ent;
//  if ((host_ent = gethostbyname(peer_ip_address_)) == 0)
//    die("Could not resolve peer host.");

  struct hostent* home_entity;
  if (!(home_entity = gethostbyname(home_ip_address_)))
    die("Failed to get home agent at specified IP");

  struct sockaddr_in peer_in;
  memset(&peer_in, 0, sizeof(peer_in));
  peer_in.sin_family = domain_;
  peer_in.sin_addr.s_addr = ((struct in_addr *)(home_entity->h_addr))->s_addr;
  peer_in.sin_port = htons(data_port_);

  if (connect(app_socket, (struct sockaddr *) &peer_in, sizeof(peer_in)))
    die("Failed to register socket at the home agent");

  return app_socket;
}

bool SimpleMobileNode::ConnectToHome(unsigned short port, char* data) {
  struct hostent* home_entity;
  if (!(home_entity = gethostbyname(home_ip_address_)))
    die("Failed to get home agent at specified IP");

  struct sockaddr_in peer_in;
  memset(&peer_in, 0, sizeof(peer_in));
  peer_in.sin_family = domain_;
  peer_in.sin_addr.s_addr = ((struct in_addr *) (home_entity->h_addr))->s_addr;
  peer_in.sin_port = htons(port);

  int connection_socket = socket(domain_, transmission_type_, protocol_);
  if (connection_socket < 0)
    die("Failed to open a socket");

  struct sockaddr_in socket_in;
  memset(&socket_in, 0, sizeof(socket_in));
  socket_in.sin_family = domain_;
  socket_in.sin_addr.s_addr = INADDR_ANY;
  socket_in.sin_port = htons(listener_port_);

  int on = 1;
  if (setsockopt(connection_socket, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<char*>(&on), sizeof(on)) < 0)
    die("Could not make the socket reusable");
  if (bind(connection_socket, (struct sockaddr*) &socket_in, sizeof(socket_in)))
    die("Error binding socket to send to home agent on port %d via %d", port,
        listener_port_);
  if (connect(connection_socket, (struct sockaddr *) &peer_in, sizeof(peer_in)))
    die("Failed to connect to the home agent");

  if (data) {
    if (send(connection_socket, data, strlen(data), 0) < 0)
      die("Error sending message to home agent");
  } else {
    fprintf(stdout, "Connected to host entity on port %d, sending via %d\n",
            port, socket_in.sin_port);
  }

  close(connection_socket);
  return true;
}

int SimpleMobileNode::GetCurrentIPAddress() const {
  struct ifaddrs *if_address, *if_struct;
  getifaddrs(&if_struct);

  for (if_address = if_struct; if_address; if_address = if_address->ifa_next) {
    struct sockaddr_in* address = (struct sockaddr_in *) if_address->ifa_addr;
    void* temp_address_pointer = &(address)->sin_addr;
    IPADDRESS(ip_string);

    inet_ntop(domain_, temp_address_pointer, ip_string, sizeof(ip_string));
    if (((address->sin_addr.s_addr << 24) >> 24) > 127) {
      freeifaddrs(if_struct);
      return address->sin_addr.s_addr;
    }
  }

  return -1;
}

bool SimpleMobileNode::ChangeHomeIdentity() {
  int current_ip_address = GetCurrentIPAddress();

  if (current_ip_address != last_known_ip_address_) {
    char buffer[33];
    snprintf(buffer, sizeof(buffer), "%-20d%-10d", last_known_ip_address_,
             listener_port_);

    ConnectToHome(change_port_, buffer);
    last_known_ip_address_ = current_ip_address;

    fprintf(stdout, "Changed IP address.  Now at %d\n", current_ip_address);
  }

  return true;
}
