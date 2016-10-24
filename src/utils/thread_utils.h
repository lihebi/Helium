#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <string>

namespace utils {
  /*******************************
   ** thread
   *******************************/
  // The timeout is SECONDs!
  // std::string exec(const char* cmd, int *status=NULL, int timeout=0);
  std::string exec(const char* cmd, int *status=NULL, double timeout=0);
  // with input
  std::string exec_in(const char* cmd, const char* input, int *status=NULL, double timeout=0);
  // std::pair<std::string, std::string> exec_both(const char* cmd, int *status=NULL, int timeout=0);
  std::string exec_sh(const char* cmd, int *status, double timeout=0);


  std::string new_exec(const char* cmd);

}

#endif /* THREAD_UTILS_H */
