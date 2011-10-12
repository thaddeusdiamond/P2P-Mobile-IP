// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// Test for a simple mobile node

#include "Common/Testing.h"
#include "MobileNode/SimpleMobileNode.h"

TEST(SimpleMobileNodeTest) {
  SimpleMobileNode* mobile_node = new SimpleMobileNode(0, 16000);
  mobile_node->Run();

  END;
}

int main(int argc, char* argv[]) {
  SimpleMobileNodeTest();
}
