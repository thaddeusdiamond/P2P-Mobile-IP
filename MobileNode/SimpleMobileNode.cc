// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is the implementation file for a simple mobile node.

#include "MobileNode/SimpleMobileNode.h"

void SimpleMobileNode::Run() {
  assert(ConnectToHome());
  do {
    sleep(1);
    assert(ChangeHomeIdentity());
    assert(InterceptConnections());
  } while (true);
}

bool SimpleMobileNode::RegisterSocket(int app_socket, int app_id) {
  application_sockets_[app_socket] = app_id;
  return true;
}

bool SimpleMobileNode::ConnectToHome() const {
  // TODO(Thad):
  //    1) Open a socket to home IP and home port, then close
  return false;
}

bool SimpleMobileNode::ChangeHomeIdentity() {
  // TODO(Thad):
  //    1) Get the current IP address
  //    2) If different than the last known identity connect to home
  return false;
}

bool SimpleMobileNode::InterceptConnections() {
  // TODO(Thad):
  //    1) Iterate through the application sockets
  //    2) If there's outgoing data on any of them scoop them up and send
  //       it to the home agent
  return false;
}
