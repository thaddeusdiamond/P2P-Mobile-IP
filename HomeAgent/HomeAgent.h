// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an interface for a home agent as specified by the Mobile IP protocol

#ifndef _P2PMIP_HOMEAGENT_HOMEAGENT_H_
#define _P2PMIP_HOMEAGENT_HOMEAGENT_H_

#include <map>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "Connectivity/types.h"

using std::map;

class HomeAgent {
 public:
  // The Home Agent constructor simply takes a transport layer type that it
  // will be using (default -- TCP)
  HomeAgent(IPAddress listener, unsigned short listener_port,
            TransportLayer transmission_type = TCP) {
    transmission_type_ = transmission_type;
    listener_ = listener;
    listener_port_ = listener_port;
  }
  virtual ~HomeAgent() {}

  // The home agent merely "runs".  We set it up initially, let it run, and it
  // accepts incoming connections, after which it spawns and subsequently kills
  // threads for connections to mobile agents.
  virtual void Run();

 protected:
  // The most important thing that a home agent does is to forward packets from
  // an incoming IP address to a specified mobile agent.
  virtual bool ForwardPackets(IPAddress recipient, Packets data);

  // If the home agent receives packets from the tunnel it must change the
  // from field in the IP address (IP layer) and forward them onto the ultimate
  // recipient.
  virtual bool RelabelPackets(IPAddress recipient, Packets data) const;

  // The home agent is responsible for adding and removing virtual tunnels
  // between itself and various mobile agents.  The tunnel will comprise an
  // <IP, outgoing Socket> pair that allows the forwarding of packets onto
  // a roaming client.
  virtual bool AddMobileAgent(IPAddress ip_address, unsigned short port);
  virtual bool RemoveMobileAgent(IPAddress ip_address);

  // The following is a map from IP values to the outgoing socket
  map<IPAddress, int> connections_;

  // We represent what type of transport this home agent is using with an enum
  int transmission_type_;

  // Finally, we represent what IP address and port the listener is sitting on
  // with class data.
  int incoming_socket_;
  IPAddress listener_;
  unsigned short listener_port_;
};

#endif  // _P2PMIP_HOMEAGENT_HOMEAGENT_H_
