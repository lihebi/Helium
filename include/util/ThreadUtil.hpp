#ifndef __THREAD_UTIL__
#define __THREAD_UTIL__

#include <cstdio>
#include <iostream>
#include <string>

class ThreadUtil {
public:
  static std::string Exec(const char* cmd);
  static std::string Exec(const std::string& cmd);
  static std::string Exec(const char* cmd, const char* input);
  static std::string Exec(const std::string& cmd, const char* input);
  static std::string Exec(const char* cmd, const std::string& input);
  static std::string Exec(const std::string& cmd, const std::string& input);
private:
};

#endif
