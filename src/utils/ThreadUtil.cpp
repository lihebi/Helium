#include "util/ThreadUtil.hpp"

std::string ThreadUtil::Exec(const char* cmd) {
  FILE* pipe = popen(cmd, "r");
  if (!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while(!feof(pipe)) {
    if(fgets(buffer, 128, pipe) != NULL) {
      result += buffer;
    }
  }
  pclose(pipe);
  return result;
}
int
ThreadUtil::ExecExit(const char* cmd) {
  FILE *pipe = popen(cmd, "r");
  if (!pipe) return -1;
  // has to have the full story to make it run
  char buffer[128];
  std::string result = "";
  while(!feof(pipe)) {
    if(fgets(buffer, 128, pipe) != NULL) {
      result += buffer;
    }
  }
  return pclose(pipe);
}
int
ThreadUtil::ExecExit(const std::string& cmd) {
  return ExecExit(cmd.c_str());
}

std::string ThreadUtil::Exec(const std::string& cmd) {
  return Exec(cmd.c_str());
}

std::string ThreadUtil::Exec(const char* cmd, const char* input) {
  FILE *pipe = popen(cmd, "r+");
  if (!pipe) return "ERROR";
  fputs(input, pipe);
  char buffer[128];
  std::string result = "";
  while(!feof(pipe)) {
    if (fgets(buffer, 128, pipe) != NULL) {
      result += buffer;
    }
  }
  pclose(pipe);
  return result;
}

std::string ThreadUtil::Exec(const char* cmd, const std::string& input) {
  return Exec(cmd, input.c_str());
}
std::string ThreadUtil::Exec(const std::string& cmd, const char* input) {
  return Exec(cmd.c_str(), input);
}
std::string ThreadUtil::Exec(const std::string& cmd, const std::string& input) {
  return Exec(cmd.c_str(), input.c_str());
}
