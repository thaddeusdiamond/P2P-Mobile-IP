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

#define IP_NAME_LENGTH 20
#define IP_PORT_LENGTH 10

#define die(...) { fprintf(stderr, __VA_ARGS__); perror(" "); exit(1); }

enum TransportLayer {
  TCP = SOCK_STREAM,
  UDP = SOCK_DGRAM,
  SCTP = IPPROTO_SCTP,
};

static inline char* trim(char* word) {
  for (unsigned int i = 0; i < strlen(word) - 1; i++) {
    if (isspace(word[i]) && isspace(word[i + 1])) {
      word[i] = '\0';
      break;
    }
  }

  int end = strlen(word) - 1;
  if (isspace(word[end]))
    word[end] = '\0';

  return word;
}

#endif  // _P2PMIP_COMMON_TYPES_H_
