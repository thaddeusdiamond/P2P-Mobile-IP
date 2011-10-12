// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is the implementation file for a simple mobile node.

#include "MobileNode/SimpleMobileNode.h"

void SimpleMobileNode::Run() {
  assert(ConnectToHome(home_port_));
  do {
    sleep(1);
    assert(ChangeHomeIdentity());
    // assert(InterceptConnections());
  } while (true);
}

bool SimpleMobileNode::RegisterSocket(int app_socket, int app_id) {
  application_sockets_[app_socket] = app_id;
  return true;
}

bool SimpleMobileNode::ConnectToHome(unsigned short port, char* data) {
  struct hostent* home_entity;
  if (!(home_entity = gethostbyname(home_ip_address_)))
    die("Failed to get home agent at specified IP");

  struct sockaddr_in peer_in;
  memset(&peer_in, 0, sizeof(peer_in));
  peer_in.sin_family = AF_INET;
  peer_in.sin_addr.s_addr = ((struct in_addr *) (home_entity->h_addr))->s_addr;
  peer_in.sin_port = htons(port);

  int connection_socket = socket(AF_INET, transmission_type_, 0);
  if (connection_socket < 0)
    die("Failed to open a socket");

  if (connect(connection_socket, (struct sockaddr *) &peer_in, sizeof(peer_in)))
    die("Failed to connect to the home agent");

  if (data) {
    if (send(connection_socket, data, strlen(data), 0) < 0)
      die("Error sending message to home agent");
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

    inet_ntop(AF_INET, temp_address_pointer, ip_string, sizeof(ip_string));
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
    snprintf(buffer, sizeof(buffer), "%d", last_known_ip_address_);

    ConnectToHome(change_port_, buffer);
    last_known_ip_address_ = current_ip_address;

    fprintf(stdout, "Changed IP address.  Now at %d\n", current_ip_address);
  }

  return true;
}

bool SimpleMobileNode::InterceptConnections() {
  // TODO(Thad):
  //    1) Iterate through the application sockets
  //    2) If there's outgoing data on any of them scoop them up and send
  //       it to the home agent
  return false;
}
