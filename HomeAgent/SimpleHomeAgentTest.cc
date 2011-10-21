// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is the test for a simple Home Agent

#include "HomeAgent/SimpleHomeAgent.h"
#include "Common/Testing.h"

TEST(SimpleHomeAgentTest, int home_port, int change_port, int data_port,
                          int next_port) {
  SimpleHomeAgent* home_agent = new SimpleHomeAgent(home_port, change_port,
                                                    data_port, next_port);
  home_agent->Run();

  END;
}

int main(int argc, char* argv[]) {
  if (argc < 5)
    die(NULL, "Usage: ./SimpleHomeAgentTest MAIN CHANGE_PORT DATA_PORT NEXT_PORT");

  SimpleHomeAgentTest(atoi(argv[1]), atoi(argv[2]),
                      atoi(argv[3]), atoi(argv[4]));
}
