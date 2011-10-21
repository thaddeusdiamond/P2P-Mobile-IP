// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// Test for a simple mobile node

#include "Common/Testing.h"
#include "MobileNode/MulticastMobileNode.h"

TEST(SimpleMobileNodeTest) {
  STATICIPADDRESS(home_ip) = "127.0.0.1";
  MulticastMobileNode* mobile_node =
    new MulticastMobileNode(home_ip, 16000, 16001, 16002, 18000);
  mobile_node->Run();

  END;
}

int main(int argc, char* argv[]) {
  SimpleMobileNodeTest();
}
