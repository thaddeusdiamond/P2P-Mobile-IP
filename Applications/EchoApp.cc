// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is the implementation for our sample echo app

#include "Applications/EchoApp.h"

void EchoApp::Run() {
  // TODO(Thad):
  //  1) Send out a single data packet
  //  2) Send back things we hear that aren't what we sent out
  assert(CreateMobileNodeDelegate());
  do {
  } while (true);
}

bool EchoApp::CreateMobileNodeDelegate() {
  // TODO(Thad):
  //  1) Spawn a new thread
  //  2) Run mobile node on that thread
  return false;
}
