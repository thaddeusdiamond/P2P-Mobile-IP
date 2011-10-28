// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is the implementation file for a simple mobile node.

#include <iostream>
#include "MobileNode/SimpleMobileNode.h"

void SimpleMobileNode::Run() {
  ConnectToHome(home_port_, NULL, true);

  Signal::HandleSignalInterrupts();
  do {
    CollectOutgoingTraffic();
    ChangeHomeIdentity();
  } while (Signal::ShouldContinue());

  ShutDown(true, "Normal termination.");
}

void SimpleMobileNode::ShutDown(bool should_exit, const char* format, ...) {
  fprintf(stdout, "Shutting Down Mobile Node... ");

  va_list arguments;
  va_start(arguments, format);
  fprintf(stderr, format, arguments);
  perror(" ");

  fprintf(stdout, "OK\n");
  if (should_exit)
    exit(1);
}

bool SimpleMobileNode::RegisterPeer(int peer_address) {
  char peer_tunnel[5] = "tun1";
  peer_fds_.insert(CreateTunnel(peer_tunnel, peer_address));
  return true;
}

int SimpleMobileNode::CreateTunnel(char* tunnel, int tunnel_address) {
  // Open the TUN clone device
  int tunnel_fd = open("/dev/net/tun", O_RDWR, 0);
  if (tunnel_fd < 0)
    ShutDown(true, "There was an error opening the virtual tunnel.");
    
  // Create a dummy virtual interface with tun properties
  struct ifreq virtual_interface;
  memset(&virtual_interface, 0, sizeof(virtual_interface));
  virtual_interface.ifr_flags = IFF_TUN;
  
  // Comment this out to get next available interface
  strncpy(virtual_interface.ifr_name, tunnel, IFNAMSIZ);

  // Perform I/O controls to set the TUN persistent, ownable and readonly/nb
  if (ioctl(tunnel_fd, TUNSETIFF, (void*) &virtual_interface) < 0)
    ShutDown(true, "Error performing ioctl on virtual tunnel");
  if (ioctl(tunnel_fd, TUNSETPERSIST, 1) < 0)
    ShutDown(true, "Could not make the tunnel persistent");
  if (ioctl(tunnel_fd, TUNSETOWNER, 777) < 0)
    ShutDown(true, "Could not set wide ownership of tunnel");
  if (fcntl(tunnel_fd, F_SETFL, O_RDONLY|O_NONBLOCK) < 0)
    ShutDown(true, "Error manipulating TUN I/O");

  strncpy(tunnel, virtual_interface.ifr_name,
          strlen(virtual_interface.ifr_name));

  // Do some extra legwork to get the TUN up, running, P2P and not using ARP
  int dummy = socket(domain_, transmission_type_, protocol_);
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, tunnel);

	if (ioctl(dummy, SIOCGIFFLAGS, &ifr) < 0)
    ShutDown(true, "Could not get I/O flags for tunnel");
  ifr.ifr_flags &= ~(IFF_BROADCAST);
	ifr.ifr_flags |= (IFF_UP | IFF_RUNNING | IFF_POINTOPOINT | IFF_NOARP);
	if (ioctl(dummy, SIOCSIFFLAGS, &ifr) < 0)
		ShutDown(true, "Could not get the tunnel up and running");

  // Lastly we give the interface an address applications can bind to
  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, tunnel);
  struct sockaddr_in *virtual_addr;
  virtual_addr = (struct sockaddr_in*) &ifr.ifr_addr;
  virtual_addr->sin_family = domain_;
  virtual_addr->sin_addr.s_addr = tunnel_address & FULL_SUBNET;
  if (ioctl(dummy, SIOCSIFADDR, &ifr) < 0)
    ShutDown(true, "Could not set interface address of tunnel");

  // Set the SUBNET Mask
  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, tunnel);
  struct sockaddr_in *virtual_netmask;
  virtual_netmask = (struct sockaddr_in*) &ifr.ifr_netmask;
  virtual_netmask->sin_family = domain_;
  virtual_netmask->sin_addr.s_addr = FULL_SUBNET;
  if (ioctl(dummy, SIOCSIFNETMASK, &ifr) < 0)
    ShutDown(true, "Could not set interface address of tunnel");
  
  // We have to double check what we set and see what the actual result was
  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, tunnel);
  if (ioctl(dummy, SIOCGIFADDR, &ifr) < 0)
    ShutDown(true, "Could not get interface address of tunnel");
  virtual_addr = (struct sockaddr_in*) &ifr.ifr_addr;
  permanent_address_ = virtual_addr->sin_addr.s_addr;

  // Output helpful scaffolding information
  std::cout << "Finished setting up TUN " << tunnel << " (" << 
    (tunnel_address & FULL_SUBNET) << ")" << std::endl;

  // Close the dummy socket we've been using and return a file descriptor to TUN
  close(dummy);
  return tunnel_fd;
}

void SimpleMobileNode::CollectOutgoingTraffic() {
  char buffer[4096];
  memset(buffer, 0, sizeof(buffer));
  int bytes_read = read(tunnel_fd_, buffer, sizeof(buffer));  

  if (bytes_read > 0) {
    std::cout << "Read (" << bytes_read << "): " << buffer + 32 << std::endl;
    write(tunnel_fd_, buffer, bytes_read);
  } else if (errno && errno != EAGAIN && errno != EWOULDBLOCK) {
    std::cout << "Goodbye, cruel world!" << std::endl; 
  }
//  else if (bytes_read < 0)
//    std::cout << "Goodbye, cruel world!" << std::endl;
/* TODO(Thad): This should collect from the virtual tunnel

  // First, we iterate through all the sockets that have been registerd
  map<int, int>::iterator it;
  for (it = app_tunnels_.begin(); it != app_tunnels_.end(); it++) {d
    char buffer[4096];
    memset(&buffer, 0, sizeof(buffer));
    
    // Next, we read from the virtual tunnels into our buffer and send along
    if (read((*it).first, buffer, sizeof(buffer)) > 0) {
      ConnectToHome(data_port_, buffer);
      fprintf(stdout, "Virtual tunnel received from application #%d: %s\n",
              (*it).second, buffer);
    }
  }*/
}

void SimpleMobileNode::ConnectToHome(unsigned short port, char* data, 
                                     bool initial) {
  // We use the newer getaddrinfo when connecting to home i/o gethostbyname  
  struct addrinfo request, *answer;
  request.ai_flags = 0;
  request.ai_family = domain_;
  request.ai_socktype = transmission_type_;
  request.ai_protocol = protocol_;

  if (getaddrinfo(home_ip_address_, NULL, &request, &answer))
    ShutDown(true, "Failed to get home agent at specified IP");

  // We want to get an IPv4 connection to the home agent to specified port
  struct sockaddr_in* in_answer = (struct sockaddr_in*) answer->ai_addr;
  in_answer->sin_port = htons(port);

  // Create a connection socket to send messages on
  int connection_socket = socket(NETv4, answer->ai_socktype,
                                 answer->ai_protocol);
  if (connection_socket < 0)
    ShutDown(true, "Failed to open a socket");

  // Use any available interface and just the listener port to send out
  // so the home agent knows where to send back to in future
  struct sockaddr_in socket_in;
  memset(&socket_in, 0, sizeof(socket_in));
  socket_in.sin_family = domain_;
  socket_in.sin_addr.s_addr = INADDR_ANY;
  socket_in.sin_port = htons(listener_port_);

  // Set all sockets to be reusable, then bind and connect
  int on = 1;
  if (setsockopt(connection_socket, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<char*>(&on), sizeof(on)) < 0)
    ShutDown(true, "Could not make the socket reusable");
  if (bind(connection_socket, (struct sockaddr*) &socket_in, sizeof(socket_in)))
    ShutDown(true, "Error binding socket to send to home agent on port %d via %d",
        port, listener_port_);
  if (connect(connection_socket, answer->ai_addr, answer->ai_addrlen))
    ShutDown(true, "Failed to connect to the home agent");
    
  // On our first time connecting to home we read back the response for our
  // permanent address and permanent port
  if (initial) {
    char buffer[4096];
    memset(&buffer, 0, sizeof(buffer));
    if (read(connection_socket, buffer, sizeof(buffer)) < 1)
      ShutDown(true, "Failed to get a permanent address from home agent.");
    
    permanent_address_ = atoi(buffer);
    permanent_port_ = atoi(buffer + 20);
    
    fprintf(stdout, "Now registered at home agent w/permanent address %d:%d\n",
            permanent_address_, permanent_port_);

    tunnel_fd_ = CreateTunnel(tunnel_name_, permanent_address_);
  }

  // Otherwise, if there's data, we send it to the home agent
  if (data) {
    if (write(connection_socket, data, strlen(data)) < 0)
      ShutDown(true, "Error sending message to home agent");
  } else {
    fprintf(stdout, "Connected to host entity on port %d, sending via %d\n",
            port, ntohs(socket_in.sin_port));
  }

  // Close connection to home
  close(connection_socket);
}

void SimpleMobileNode::ChangeHomeIdentity() {
  int current_ip_address = GetCurrentIPAddress();

  if (current_ip_address != last_known_ip_address_) {
    char buffer[31];
    snprintf(buffer, sizeof(buffer), "%-20d%-10d", last_known_ip_address_,
             listener_port_);

    ConnectToHome(change_port_, buffer);
    last_known_ip_address_ = current_ip_address;

    fprintf(stdout, "Changed IP address.  Now at %d\n", current_ip_address);
  }
}
