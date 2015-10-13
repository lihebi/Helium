#include "util/ThreadUtil.hpp"
#include <cstring>
#include <unistd.h>

std::string ThreadUtil::Exec(const char* cmd) {
  FILE* pipe = popen(cmd, "r");
  if (!pipe) return "[ThreadUtil::Exec] ERROR";
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

std::string ThreadUtil::Exec(const char* cmd, const std::string& input, unsigned int timeout) {
  return Exec(cmd, input.c_str(), timeout);
}
std::string ThreadUtil::Exec(const std::string& cmd, const char* input, unsigned int timeout) {
  return Exec(cmd.c_str(), input, timeout);
}
std::string ThreadUtil::Exec(const std::string& cmd, const std::string& input, unsigned int timeout) {
  return Exec(cmd.c_str(), input.c_str(), timeout);
}

int split(const char *scon, char** &argv) {
  char *s = strdup(scon);
  char *tok = strtok(s, " ");
  argv = (char**)malloc(10*sizeof(char*));
  int argc = 0;
  while (tok) {
    argv[argc] = (char*)malloc(strlen(tok)+1);
    strcpy(argv[argc], tok);
    tok = strtok(NULL, " ");
    argc++;
  }
  argv[argc] = NULL;
  return argc;
}

std::string
ThreadUtil::Exec(const char* cmd, const char* input, unsigned int timeout) {
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
    perror("fork");
    exit(1);
  }
  if (pid == 0) {
    // children
    if (timeout>0) alarm(timeout);
    if (dup2(pipein[0], 0) == -1 || dup2(pipeout[1], 1) == -1) {
      perror("dup error");
      exit(1);
    }
    close(pipein[0]);
    close(pipein[1]);
    close(pipeout[0]);
    close(pipeout[1]);
    // prepare the command
    char **argv = NULL;
    // the argv is malloc-ed, but anyway the process will exit, it will be released
    split(cmd, argv);
    execvp(argv[0], argv);
    perror("execvp");
    exit(1);
  }
  // parent
  close(pipein[0]);
  close(pipeout[1]);
  // write to child's input
  write(pipein[1], input, strlen(input));
  close(pipein[1]);
  char buf[BUFSIZ];
  int nread;
  while ((nread = read(pipeout[0], buf, sizeof(buf))) != 0) {
    if (nread == -1) {
      perror("read");
    } else {
      result.append(buf, nread);
    }
  }
  close(pipeout[0]);
  return result;
}
