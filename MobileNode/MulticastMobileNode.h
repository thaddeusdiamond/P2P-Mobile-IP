// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is a type of mobile node that communicates via multicast in Mobile IP.

#ifndef _P2PMIP_MOBILENODE_MULTICASTMOBILENODE_H_
#define _P2PMIP_MOBILENODE_MULTICASTMOBILENODE_H_

#include "MobileNode/SimpleMobileNode.h"

class MulticastMobileNode : public SimpleMobileNode {
 public:
  MulticastMobileNode(IPAddress home_ip_address, unsigned short home_port,
                   unsigned short change_port, unsigned short data_port,
                   unsigned short listener_port,
                   TransportLayer transmission_type = TCP,
                   Domain domain = NET, Protocol protocol = NO_TYPE) :
    SimpleMobileNode(home_ip_address, home_port, change_port, data_port,
                     listener_port) {}

  virtual ~MulticastMobileNode() {}

  // The only thing that's special about multicast mobile nodes is how we
  // intercept sockets
  virtual int CreateTunnel(char* tunnel_name);
};

#endif  // _P2PMIP_MOBILENODE_MULTICASTMOBILENODE_H_
