// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is a standard header file for various connectivity switches

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#define IPAddress std::string
#define Packets std::string

enum TransportLayer {
  TCP = SOCK_STREAM,
  UDP = SOCK_DGRAM,
  SCTP = IPPROTO_SCTP,
};
