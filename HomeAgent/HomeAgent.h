// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is an abstract interface for any home agent as specified by the
// Mobile IP protocol

#ifndef _P2PMIP_HOMEAGENT_HOMEAGENT_H_
#define _P2PMIP_HOMEAGENT_HOMEAGENT_H_

class HomeAgent {
 public:
  virtual ~HomeAgent() {}

  // Server-like "run" paradigm
  virtual void Run() = 0;

 protected:
  // Forwarding incoming packets from the tunnel
  virtual bool ForwardPackets(int tunnel) = 0;

  // Relabeling outgoing packets and sending them
  virtual bool RelabelPackets(int outbound) = 0;

  // Adding and removing mobile agents
  virtual bool AddMobileAgent(unsigned short out_port, int socket) = 0;
  virtual bool RemoveMobileAgent(int tunnel) = 0;

  // Change mobile agent's identity
  virtual bool ChangeMobileAgent(int tunnel) = 0;
};

#endif  // _P2PMIP_HOMEAGENT_HOMEAGENT_H_
