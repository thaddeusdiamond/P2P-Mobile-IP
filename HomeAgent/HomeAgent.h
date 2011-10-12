// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an interface for a home agent as specified by the Mobile IP protocol

#ifndef _P2PMIP_HOMEAGENT_HOMEAGENT_H_
#define _P2PMIP_HOMEAGENT_HOMEAGENT_H_

#include <map>
#include <list>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno> 
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include "Common/Types.h"

#define MAX_CONNECTIONS 64

using std::map;
using std::list;

class HomeAgent {
 public:
  // The Home Agent constructor simply takes a transport layer type that it
  // will be using (default -- TCP)
  HomeAgent(unsigned short listener_port, unsigned short next_available_port,
            TransportLayer transmission_type = TCP) {
    transmission_type_ = transmission_type;
    listener_port_ = listener_port;
    next_available_port_ = next_available_port;
  }
  virtual ~HomeAgent() {}

  // The home agent merely "runs".  We set it up initially, let it run, and it
  // accepts incoming connections, after which it spawns and subsequently kills
  // threads for connections to mobile agents.
  virtual void Run();

 protected:
  // First, a home agent must create the socket to listen for new connections
  int CreateSocket(unsigned short port) const;
  
  // The most important thing that a home agent does is to forward packets from
  // an incoming IP address to a specified mobile agent.
  virtual bool ForwardPackets(int tunnel);

  // If the home agent receives packets from the tunnel it must change the
  // from field in the IP address (IP layer) and forward them onto the ultimate
  // recipient.
  virtual bool RelabelPackets(int outbound) const;

  // The home agent is responsible for adding and removing virtual tunnels
  // between itself and various mobile agents.  The tunnel will comprise an
  // <IP, outgoing Socket> pair that allows the forwarding of packets onto
  // a roaming client.
  virtual int AddMobileAgent(unsigned short port, int socket);
  virtual bool RemoveMobileAgent(int tunnel);

  // The following is a map from incoming ports to virtual tunnels, and vice
  // versa for fast lookup and forwarding.
  map<int, int> connections_in_;
  map<int, int> connections_out_;
  
  // We maintain the highest socket yet opened for use with select().
  int fd_limit_;

  // We represent what type of transport this home agent is using with an enum
  int transmission_type_;
  
  // We need to represent what the next available port to delegate out is
  unsigned short next_available_port_;

  // Finally, we represent what port the home agent takes new connections on.
  unsigned short listener_port_;
};

#endif  // _P2PMIP_HOMEAGENT_HOMEAGENT_H_
