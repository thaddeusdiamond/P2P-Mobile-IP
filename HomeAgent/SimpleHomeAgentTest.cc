// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is the test for a simple Home Agent

#include "HomeAgent/SimpleHomeAgent.h"
#include "Common/Testing.h"

TEST(SimpleHomeAgentTest) {
  SimpleHomeAgent* home_agent = new SimpleHomeAgent(16000, 16001, 16002, 16003);
  home_agent->Run();

  END;
}

int main(int argc, char* argv[]) {
  SimpleHomeAgentTest();
}
