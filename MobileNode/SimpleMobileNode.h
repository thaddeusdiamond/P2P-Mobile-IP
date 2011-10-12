// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an abstract interface for a simple implementation of the mobile node
// in Mobile IP

#ifndef _P2PMIP_MOBILENODE_SIMPLEMOBILENODE_H_
#define _P2PMIP_MOBILENODE_SIMPLEMOBILENODE_H_

#include <cassert>
#include <map>
#include "Common/Types.h"
#include "MobileNode/MobileNode.h"

using std::map;

class SimpleMobileNode : public MobileNode {
 public:
  explicit SimpleMobileNode(int home_ip_address, unsigned short home_port) {
    home_ip_address_ = home_ip_address;
    home_port_ = home_port;
    // TODO(Thad): Set last known IP address
  }
  virtual ~SimpleMobileNode() {}

  // We use a daemon-like "run" paradigm
  virtual void Run();

  // Any application needs to register an open socket so that it can be
  // intercepted
  virtual bool RegisterSocket(int app_socket, int app_id);

 protected:
  // A mobile agent needs to instantiate a connection to the home agent
  virtual bool ConnectToHome() const;

  // A mobile node needs to update the home agent when it's IP changes
  virtual bool ChangeHomeIdentity();

  // The mobile node daemon intercepts outbound socket connections
  virtual bool InterceptConnections();

  // Listed connection to home IP address
  int home_ip_address_;
  unsigned short home_port_;

  // We keep the last known identity and change the interface if that's
  // no longer our IP
  int last_known_ip_address_;

  // A map from registered sockets to the application that owns them
  map<int, int> application_sockets_;
};

#endif  // _P2PMIP_MOBILENODE_SIMPLEMOBILENODE_H_
