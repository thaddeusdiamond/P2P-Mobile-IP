// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an abstract interface for a simple implementation of the mobile node
// in Mobile IP

#ifndef _P2PMIP_MOBILENODE_SIMPLEMOBILENODE_H_
#define _P2PMIP_MOBILENODE_SIMPLEMOBILENODE_H_

#include <netdb.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <map>
#include "Common/Types.h"
#include "MobileNode/MobileNode.h"

using std::map;

class SimpleMobileNode : public MobileNode {
 public:
  explicit SimpleMobileNode(IPADDRESS(home_ip_address),
                            unsigned short home_port,
                            unsigned short change_port,
                            unsigned short data_port,
                            TransportLayer transmission_type = TCP) {
    home_port_ = home_port;
    change_port_ = change_port;
    data_port_ = data_port;

    transmission_type_ = transmission_type;

    strncpy(home_ip_address_, home_ip_address, sizeof(home_ip_address_));
    last_known_ip_address_ = GetCurrentIPAddress();
  }
  virtual ~SimpleMobileNode() {}

  // We use a daemon-like "run" paradigm
  virtual void Run();

  // Any application needs to register an open socket so that it can be
  // intercepted
  virtual bool RegisterSocket(int app_socket, int app_id);

 protected:
  // A mobile agent needs to instantiate a connection to the home agent
  virtual bool ConnectToHome(unsigned short port, char* data = NULL);

  // A mobile node needs to update the home agent when it's IP changes
  virtual int GetCurrentIPAddress() const;
  virtual bool ChangeHomeIdentity();

  // Listed connection to home IP address
  IPADDRESS(home_ip_address_);
  unsigned short home_port_;
  unsigned short change_port_;
  unsigned short data_port_;

  // We keep the last known identity and change the interface if that's
  // no longer our IP
  int last_known_ip_address_;

  // We must specify what type of transport layer protocol we are using
  int transmission_type_;

  // A map from registered sockets to the application that owns them
  map<int, int> application_sockets_;
};

#endif  // _P2PMIP_MOBILENODE_SIMPLEMOBILENODE_H_
