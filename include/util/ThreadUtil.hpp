#ifndef __THREAD_UTIL__
#define __THREAD_UTIL__

#include <cstdio>
#include <iostream>
#include <string>

class ThreadUtil {
public:
  // TODO return timeout as status
  static std::string Exec(const char* cmd);
  static std::string Exec(const std::string& cmd);
  // return exit code
  static int ExecExit(const char* cmd);
  static int ExecExit(const std::string& cmd);
  // with input
  static std::string Exec(const char* cmd, const char* input, unsigned int timeout = 0);
  static std::string Exec(const std::string& cmd, const char* input, unsigned int timeout = 0);
  static std::string Exec(const char* cmd, const std::string& input, unsigned int timeout = 0);
  static std::string Exec(const std::string& cmd, const std::string& input, unsigned int timeout = 0);
private:
};

#endif
