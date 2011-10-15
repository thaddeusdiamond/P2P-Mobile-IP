// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is a test for our simple echo application

#include "Common/Testing.h"
#include "Applications/EchoApp.h"

TEST(EchoAppTest) {
  char keyword[20] = "Disco!";
  IPADDRESS(peer_ip) = "127.0.0.1";
  EchoApp* application = new EchoApp(keyword, peer_ip, 17000, 18000);
  application->Run();

  END;
}

int main(int argc, char* argv[]) {
  EchoAppTest();
}
