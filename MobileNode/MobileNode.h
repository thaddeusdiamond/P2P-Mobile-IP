// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an abstract interface for a mobile node traveling on the network

#ifndef _P2PMIP_MOBILENODE_MOBILENODE_H_
#define _P2PMIP_MOBILENODE_MOBILENODE_H_

class MobileNode {
 public:
  virtual ~MobileNode() {}

  // We use a daemon-like "run" paradigm
  virtual void* Run() = 0;

  // Any application needs to register an open socket so that it can be
  // intercepted
  virtual int RegisterSocket(int app_socket, int app_id,
                              IPADDRESS(peer_ip_address),
                              unsigned short peer_port) = 0;

 protected:
  // A mobile agent needs to instantiate a connection to the home agent
  virtual int GetCurrentIPAddress() const = 0;
  virtual bool ConnectToHome(unsigned short port, char* data) = 0;

  // A mobile node needs to update the home agent when it's IP changes
  virtual bool ChangeHomeIdentity() = 0;
};

#endif  // _P2PMIP_MOBILENODE_MOBILENODE_H_
