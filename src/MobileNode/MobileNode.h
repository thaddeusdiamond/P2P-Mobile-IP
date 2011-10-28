// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an abstract interface for a mobile node traveling on the network

#ifndef _P2PMIP_MOBILENODE_MOBILENODE_H_
#define _P2PMIP_MOBILENODE_MOBILENODE_H_

class MobileNode {
 public:
  virtual ~MobileNode() {}

  // We use a daemon-like "run" and "shutdown" paradigm
  virtual void Run() = 0;
  virtual void ShutDown(bool should_exit, const char* format, ...) = 0;

  // We need an interface for an application to know where to listen in on
  virtual int GetPermanentAddress() = 0;

  // Any application needs to register an open socket so that it can be
  // intercepted and virtually tunneled
  virtual bool RegisterPeer(int peer_address) = 0;

 protected:
  // A mobile agent needs to instantiate a connection to the home agent
  virtual void ConnectToHome(unsigned short port, char* data, bool initial) = 0;

  // A mobile node needs to update the home agent when it's IP changes
  virtual void ChangeHomeIdentity() = 0;
};

#endif  // _P2PMIP_MOBILENODE_MOBILENODE_H_
