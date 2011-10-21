// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an abstract interface for any application that wants to run using
// Mobile IP

#ifndef _P2PMIP_APPLICATIONS_APPLICATION_H_
#define _P2PMIP_APPLICATIONS_APPLICATION_H_

class Application {
 public:
  virtual ~Application() {}

  // Application-like "run" and "shutdown" paradigm
  virtual void Run() = 0;
  virtual void ShutDown(const char* format, ...) = 0;

 protected:
  // Create a mobile node to monitor our location/update home agent
  virtual int CreateMobileNodeDelegate() = 0;
};

#endif  // _P2PMIP_APPLICATIONS_APPLICATION_H_
