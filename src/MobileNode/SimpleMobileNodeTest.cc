// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// Test for a simple mobile node

#include "Common/Testing.h"
#include "MobileNode/SimpleMobileNode.h"

TEST(SimpleMobileNodeTest, IPAddress home_ip, int home_port, int change_port,
                           int data_port, int listener) {
  SimpleMobileNode* mobile_node =
    new SimpleMobileNode(home_ip, home_port, change_port, data_port, listener);
  mobile_node->Run();

  END;
}

int main(int argc, char* argv[]) {
  if (argc < 6)
    die(NULL, "Usage: ./SimpleMobileNodeTest HOME_IP PORT CHANGE_PORT DATA_PORT MOBILE_PORT");

  SimpleMobileNodeTest(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]),
                       atoi(argv[5]));
}
