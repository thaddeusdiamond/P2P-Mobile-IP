// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is a test for our simple echo application

#include "Common/Testing.h"
#include "Applications/EchoApp.h"

TEST(EchoAppTest, char* keyword, int listener,
                  IPAddress home_ip, int home_port, int change_port,
                  int data_port, IPAddress peer_ip, int peer_port) {
  EchoApp* application = new EchoApp(keyword, home_ip, home_port, change_port,
                                     data_port, peer_ip, peer_port, listener);
  application->Run();

  END;
}

int main(int argc, char* argv[]) {
  if (argc < 5)
    die(NULL, "Usage: ./EchoAppTest KEYWORD LISTENER_PORT HOME_IP PORT CHANGE_PORT DATA_PORT PEER_IP PORT");

  EchoAppTest(argv[1], atoi(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]),
                       atoi(argv[6]), argv[7], atoi(argv[8]));
}
