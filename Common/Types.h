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
#include <cstdlib>
#include <cctype>
#include <cstdio>
#include <cstdarg>

#define IPAddress char*
#define STATICIPADDRESS(MSG) char MSG[INET_ADDRSTRLEN + 1]
#define Packets std::string

#define IP_NAME_LENGTH 20
#define IP_PORT_LENGTH 10

enum TransportLayer {
  RAW = SOCK_RAW,
  TCP = SOCK_STREAM,
  UDP = SOCK_DGRAM,
  SCTP = SOCK_SEQPACKET,
};

enum Domain {
  NET = PF_INET,
};

enum Protocol {
  NO_TYPE = 0,
  SCTP_PROTO = IPPROTO_SCTP,
};

static inline void die(void* callback_function, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  fprintf(stderr, format, arguments);

  perror(" ");
  exit(1);
}

static inline char* trim(char* word) {
  if (!word || !word[0])
    return NULL;

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
