// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is a sample echo app that merely opens a connection to a peer and begins
// a two-way echo chamber (assuming the peer is also running the echo app)

#ifndef _P2PMIP_APPLICATIONS_ECHOAPP_H_
#define _P2PMIP_APPLICATIONS_ECHOAPP_H_

#include <fcntl.h>
#include <pthread.h>
#include <cassert>
#include "Common/Signal.h"
#include "Common/Types.h"
#include "MobileNode/SimpleMobileNode.h"
#include "Applications/Application.h"

#define MAX_CONNECTIONS 64

class EchoApp : public Application {
 public:
  EchoApp(char* keyword,
          IPAddress home_ip_address, unsigned short home_port,
          unsigned short change_port, unsigned short data_port,
          IPAddress peer_ip_address, unsigned short peer_port,
          unsigned short listener_port, TransportLayer transmission_type = TCP,
          Domain domain = NET, Protocol protocol = NO_TYPE) {
    keyword_ = keyword;
    listener_port_ = listener_port;

    peer_ip_address_ = peer_ip_address;
    peer_port_ = peer_port;

    home_ip_address_ = home_ip_address;
    home_port_ = home_port;
    change_port_ = change_port;
    data_port_ = data_port;

    transmission_type_ = transmission_type;
    domain_ = domain;
    protocol_ = protocol;
  }
  virtual ~EchoApp() {}

  // Application-like "run" paradigm
  virtual void Run();

  // Need to have a "shutdown" method that can be called with a SIGINT handler
  virtual void ShutDown(const char* format, ...);

 protected:
  // This is the method that creates an instance of our mobile node delegate on
  // a thread so that we can register sockets with it and follow the Mobile IP
  // protocol
  virtual int CreateMobileNodeDelegate();

  // We delegate a special create socket method for this type of app.  Although
  // generally desirable, it is not strictly required for an application and
  // is therefore left not virtual.
  int CreateSocket(bool listener = false);

  // Whenever someone sends us data we want to have an abstracted manner of
  // handling that so we don't clog up the application pool.  We do this for
  // sending out data as well.
  void PrintReceivedData();
  void EchoMessage(char* message, char* peer_ip_address = NULL, 
                   int peer_ip_port = 0);

  // We keep track of the keyword we are going to send out to the server so that
  // we don't de-dup our traffic.  We make the assumption no two echo
  // applications have the same keyword.
  char* keyword_;

  // We need to listen in on a port for traffic from our peers.
  int listener_socket_;
  unsigned short listener_port_;

  // We also want to keep track of what host and port number we are
  // communicating with.  We instantiate the socket normally, but later register
  // it with the mobile node agent, which manipulates it's behavior.
  IPAddress peer_ip_address_;
  unsigned short peer_port_;

  // We specify how we communicate with other people (by default TCP).
  TransportLayer transmission_type_;
  Domain domain_;
  Protocol protocol_;

  // Finally, we need to know what host we are going to be communicating to over
  // Mobile IP.  This is just the ultimate home router (things may become
  // more complex with hierarchical name routing).
  IPAddress home_ip_address_;
  unsigned short home_port_;
  unsigned short change_port_;
  unsigned short data_port_;

  // Each application owns an instance of the mobile node daemon.  This is for
  // practical purposes of implementing a single mobile node agent with
  // sharable memory (and trying to avoid interprocess communication).  Better
  // implementations would look at interprocess communication or multiple
  // applications on the same process with thread-safe mobile_node_ pointers.
  MobileNode* mobile_node_;
};

static inline void* RunMobileAgentThread(void* agent) {
  (reinterpret_cast<SimpleMobileNode *>(agent))->Run();
  return NULL;
}

#endif  // _P2PMIP_APPLICATIONS_ECHOAPP_H_
