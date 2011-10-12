// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// Test for a simple mobile node

#include "Common/Testing.h"
#include "MobileNode/SimpleMobileNode.h"

TEST(SimpleMobileNodeTest) {
  char home_ip[100] = "127.0.0.1";
  SimpleMobileNode* mobile_node =
    new SimpleMobileNode(home_ip, 16000, 16001, 16002);
  mobile_node->Run();

  END;
}

int main(int argc, char* argv[]) {
  SimpleMobileNodeTest();
}
