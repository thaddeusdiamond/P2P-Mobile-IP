// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is a sample echo app that merely opens a connection to a peer and begins
// a two-way echo chamber (assuming the peer is also running the echo app)

#ifndef _P2PMIP_APPLICATIONS_ECHOAPP_H_
#define _P2PMIP_APPLICATIONS_ECHOAPP_H_

#include <cassert>
#include "MobileNode/MobileNode.h"
#include "Applications/Application.h"

class EchoApp : public Application {
 public:
  EchoApp() {}
  virtual ~EchoApp() {}

  // Application-like "run" paradigm
  virtual void Run();

 protected:
  // This is the method that creates an instance of our mobile node delegate on
  // a thread so that we can register sockets with it and follow the Mobile IP
  // protocol
  virtual bool CreateMobileNodeDelegate();

  MobileNode* mobile_node_;
};

#endif  // _P2PMIP_APPLICATIONS_ECHOAPP_H_
