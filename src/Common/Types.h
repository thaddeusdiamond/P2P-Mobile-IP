// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is a standard header file for various connectivity switches

#ifndef _P2PMIP_COMMON_TYPES_H_
#define _P2PMIP_COMMON_TYPES_H_

#include <iostream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
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
#define IPv6_ADDR_LENGTH 16

#define FULL_SUBNET 16777215

enum TransportLayer {
  RAW = SOCK_RAW,
  TCP = SOCK_STREAM,
  UDP = SOCK_DGRAM,
  SCTP = SOCK_SEQPACKET,
};

enum Domain {
  PACKET = AF_PACKET,
  NETv4 = PF_INET,
  NETv6 = PF_INET6,
};

enum Protocol {
  NO_TYPE = 0,
  UDP_PROTO = IPPROTO_UDP,
  SCTP_PROTO = IPPROTO_SCTP,
};

static inline void Die(void* callback_function, const char* format, ...) {
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

static inline int GetCurrentIPAddress() {
  struct ifaddrs *if_address, *if_struct;
  getifaddrs(&if_struct);

  for (if_address = if_struct; if_address != NULL; 
       if_address = if_address->ifa_next) {
    struct sockaddr_in* address = (struct sockaddr_in*) if_address->ifa_addr;
    struct sockaddr_in* netmask = (struct sockaddr_in*) if_address->ifa_netmask;
    struct sockaddr_in* broad = (struct sockaddr_in*) if_address->ifa_ifu.ifu_broadaddr;
    struct sockaddr_in* dst = (struct sockaddr_in*) if_address->ifa_ifu.ifu_dstaddr;

    std::cout << if_address->ifa_name << std::endl;
    if (address != NULL) {
      std::cout << " ADDR(" << address->sin_addr.s_addr << ":" << 
        address->sin_port << ")" << std::endl;
    }
    if (netmask != NULL) {
      std::cout << " NETM(" << netmask->sin_addr.s_addr << ":" << 
        netmask->sin_port << ")" << std::endl;
    }
    if (broad != NULL) {
      std::cout << " BROAD(" << broad->sin_addr.s_addr << ":" << 
        broad->sin_port << ")" << std::endl;
    }
    if (dst != NULL) {
      std::cout << " DST(" << dst->sin_addr.s_addr << ":" << 
        dst->sin_port << ")" << std::endl;
    }

    if (address != NULL && ((address->sin_addr.s_addr << 24) >> 24) > 127) {
      int ip_number = address->sin_addr.s_addr;
      freeifaddrs(if_struct);
      return ip_number;
    }
  }

  return -1;
}

#endif  // _P2PMIP_COMMON_TYPES_H_
