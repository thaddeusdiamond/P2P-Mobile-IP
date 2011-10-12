// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is a standard header file for various connectivity switches

#ifndef _P2PMIP_COMMON_TYPES_H_
#define _P2PMIP_COMMON_TYPES_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <cstring>
#include <cctype>

#define IPADDRESS(MSG) char MSG[INET_ADDRSTRLEN]
#define Packets std::string

#define die(MSG) { perror(MSG); exit(1); }

enum TransportLayer {
  TCP = SOCK_STREAM,
  UDP = SOCK_DGRAM,
  SCTP = IPPROTO_SCTP,
};

static inline char* trim(char* word) {
  for (unsigned int i = 0; i < strlen(word); i++) {
    if (isspace(word[i])) {
      word[i] = '\0';
      break;
    }
  }

  return word;
}

#endif  // _P2PMIP_COMMON_TYPES_H_
