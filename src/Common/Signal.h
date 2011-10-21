// Author: Thaddeus Diamond (diamond@cs.yale.edu)
//
// This is a standard header file for static functions handling SIGINTs

#ifndef _P2PMIP_COMMON_SIGNAL_H_
#define _P2PMIP_COMMON_SIGNAL_H_

#include <signal.h>

namespace Signal {
  static bool exit_flag = false;

  static inline bool ShouldContinue() {
    return !exit_flag;
  }

  static inline void ExitProgram(int parameter) {
    exit_flag = true;
  }

  static inline void HandleSignalInterrupts() {
    signal(SIGINT, &ExitProgram);
    signal(SIGTERM, &ExitProgram);
  }
}

#endif  // _P2PMIP_COMMON_SIGNAL_H_
