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

// std::string ThreadUtil::Exec(const char* cmd, const char* input) {
//   std::cout << "[ThreadUtil::Exec]" << std::endl;
//   FILE *pipe = popen(cmd, "r+");
//   if (!pipe) return "ERROR";
//   fputs(input, pipe);
//   char buffer[128];
//   std::string result = "";
//   while(!feof(pipe)) {
//     std::cout << "...." << std::endl;
//     if (fgets(buffer, 128, pipe) != NULL) {
//       result += buffer;
//     }
//     std::cout << "/* message */" << std::endl;
//   }
//   std::cout << "////" << std::endl;
//   pclose(pipe);
//   return result;
// }

std::string ThreadUtil::Exec(const char* cmd, const std::string& input) {
  return Exec(cmd, input.c_str());
}
std::string ThreadUtil::Exec(const std::string& cmd, const char* input) {
  return Exec(cmd.c_str(), input);
}
std::string ThreadUtil::Exec(const std::string& cmd, const std::string& input) {
  return Exec(cmd.c_str(), input.c_str());
}

std::string
ThreadUtil::Exec(const char* cmd, const char* input) {
  // cmd should not exceed BUFSIZ
  char cmd_buf[BUFSIZ];
  strcpy(cmd_buf, cmd);
  std::string result;
  // create pipe
  int pipein[2], pipeout[2];
  if (pipe(pipein) == -1 || pipe(pipeout) == -1) {
    perror("pipe");
    exit(1);
  }
  pid_t pid;
  // fork
  pid = fork();
  if (pid<0) {
    std::cerr << "fork failed" << std::endl;
  }
  if (pid == 0) {
    // children
    if (dup2(pipein[0], 0) == -1 || dup2(pipeout[1], 1) == -1) {
      perror("dup error");
      exit(1);
    }
    close(pipein[0]);
    close(pipein[1]);
    close(pipeout[0]);
    close(pipeout[1]);
    // prepare the command
    char *args;
    args = strchr(cmd_buf, ' ');
    if (args) {
      *args = '\0';
      args++;
    }
    execlp(cmd_buf, cmd_buf, args);
    perror(cmd_buf);
    exit(1);
  }
  // parent
  close(pipein[0]);
  close(pipeout[1]);
  // write to child's input
  write(pipein[1], input, strlen(input));
  close(pipein[1]);
  char buf[BUFSIZ];
  while (1) {
    // read from child's output
    int nread = read(pipeout[0], buf, sizeof(buf));
    if (nread == -1) {
      // error
    } else if (nread == 0) {
      break;
    } else {
      result += buf;
    }
  }
  close(pipeout[0]);
  return result;
}
