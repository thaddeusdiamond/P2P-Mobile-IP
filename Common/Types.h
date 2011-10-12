// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is a standard header file for various connectivity switches

#include <string>
#include <sys/types.h>
#include <sys/socket.h>

#define IPAddress char*
#define Packets std::string

enum TransportLayer {
  TCP = SOCK_STREAM,
  UDP = SOCK_DGRAM,
  SCTP = IPPROTO_SCTP,
};
