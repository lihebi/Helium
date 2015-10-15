#ifndef __THREAD_UTIL__
#define __THREAD_UTIL__

#include <cstdio>
#include <iostream>
#include <string>

class ThreadUtil {
public:
  // TODO return timeout as status
  static std::string Exec(
    const char* cmd,
    int *status,
    int timeout=0
  );
  // with input
  static std::string Exec(
    const char* cmd,
    const char* input,
    int *status,
    unsigned int timeout=0
  );
private:
};

#endif
