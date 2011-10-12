// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is the test for a simple Home Agent

#include "HomeAgent/HomeAgent.h"
#include "Common/Testing.h"

TEST(HomeAgentTest) {
  HomeAgent* home_agent = new HomeAgent(8000, 8001);
  home_agent->Run();
  
  END;
}

int main() {
  HomeAgentTest();
}
