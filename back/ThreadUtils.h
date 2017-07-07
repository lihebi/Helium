#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <string>
#include <sys/time.h>
#include <cmath>
#include <iostream>

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


class ThreadExecutor {
public:
  ThreadExecutor(std::string cmd) : cmd(cmd) {}
  ~ThreadExecutor() {}
  void setInput(std::string input) {
    this->input = input;
  }
  void setTimeoutSec(double timeout) {
    timeout_sec = timeout;
    use_timeout = true;
  }
  void run();
  int getReturnCode() {return ReturnCode;}
  bool isTimedOut() {return TimedOut;}
  std::string getStdOut() {return stdout;}
  // TODO
  std::string getStdErr() {return stderr;}
  double getExecutionTime() {
    return UsedTime;
  }
  void handle_signal(int signal);

  void child(int p0[2], int p1[2]);
  void parent(int child_pid, int p0[2], int p1[2]);
  bool checkChildStatus(int child_pid);
private:
  std::string cmd;
  std::string input;
  // only >0 will take effect
  double timeout_sec = 0;
  bool use_timeout = false;
  // double remaining_timeout = 0;
  // struct itimerval *timer = {0};

  bool TimedOut = false;
  int ReturnCode = -1;
  std::string stdout;
  std::string stderr;
  double UsedTime = 0;
};

#endif /* THREAD_UTILS_H */
