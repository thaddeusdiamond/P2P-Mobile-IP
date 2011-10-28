// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an interface for a home agent as specified by the Mobile IP protocol

#ifndef _P2PMIP_HOMEAGENT_SIMPLEHOMEAGENT_H_
#define _P2PMIP_HOMEAGENT_SIMPLEHOMEAGENT_H_

#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <list>
#include <cerrno>
#include "Common/Types.h"
#include "Common/Signal.h"
#include "HomeAgent/HomeAgent.h"

#define MAX_CONNECTIONS 64

using std::map;
using std::list;

class SimpleHomeAgent : public HomeAgent {
 public:
  // The Home Agent constructor simply takes a transport layer type that it
  // will be using (default -- TCP)
  SimpleHomeAgent(unsigned short listener_port, unsigned short change_port,
                  unsigned short data_port, unsigned short next_port,
                  TransportLayer transmission_type = TCP, Domain domain = NETv4,
                  Protocol protocol = NO_TYPE) {
    transmission_type_ = transmission_type;
    domain_ = domain;
    protocol_ = protocol;

    listener_port_ = listener_port;
    change_port_ = change_port;
    data_port_ = data_port;
    next_port_ = next_port;
  }
  virtual ~SimpleHomeAgent() {}

  // The home agent merely "runs".  We set it up initially, let it run, and it
  // accepts incoming connections, after which it spawns and subsequently kills
  // threads for connections to mobile agents.
  virtual void Run();

  // The home agent has a shutdown method that can be called so that a signal
  // interruption gracefully shuts down the process.  This will close all
  // open connections.
  virtual void ShutDown(const char* format, ...);

 protected:
  // The home agent is responsible for adding and removing virtual tunnels
  // between itself and various mobile agents.  The tunnel will comprise an
  // <IP, outgoing Socket> pair that allows the forwarding of packets onto
  // a roaming client.
  virtual bool AddMobileAgent(unsigned short out_port, int socket);
  virtual bool RemoveMobileAgent(int tunnel);

  // We sometimes get a new connection that tells us how to change a mobile
  // connection
  virtual bool ChangeMobileAgent(int tunnel);

  // The most important thing that a home agent does is to forward packets from
  // an incoming IP address to a specified mobile agent.
  virtual bool ForwardPackets(int tunnel);

  // If the home agent receives packets from the tunnel it must change the
  // from field in the IP address (IP layer) and forward them onto the ultimate
  // recipient.
  virtual bool RelabelPackets(int outbound);

  // First, a home agent must create the socket to listen for new connections
  int CreateSocket(unsigned short port, bool listen = true,
                   bool nonblocking = true, int* ip_address = NULL);

  // The following is a map from incoming ports to IP addresses
  // (virtual tunnels).
  map<int, list<int> > connections_in_;

  // We need to maintain a map of the tunnel sockets to the identities to
  // prevent unauthorized spoofing
  map<int, int> tunnel_identities_;

  // Keep a list of the three main ports
  int listening_socket_;
  int change_socket_;
  int data_socket_;

  // We maintain the highest socket yet opened for use with select().
  int fd_limit_;

  // We represent what type of transport this home agent is using with an enum
  TransportLayer transmission_type_;
  Domain domain_;
  Protocol protocol_;

  // We need to represent what the next available port to delegate out is
  unsigned short next_port_;

  // Finally, we represent what port the home agent takes new connections on,
  // which port it looks for changes in identity, and which port accepts data
  // from mobile nodes.
  unsigned short listener_port_;
  unsigned short change_port_;
  unsigned short data_port_;
};

#endif  // _P2PMIP_HOMEAGENT_SIMPLEHOMEAGENT_H_
