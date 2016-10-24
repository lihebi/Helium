#include "utils.h"
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#include <fcntl.h>

#include <gtest/gtest.h>

#define TIMEOUT_RET_CODE 1234

// FIXME: Since I changed the timeout to be microseconds, all the commands use this need to change the time accordingly.
// FIXME: I changed exec to use execvp, and added another exec_sh to use sh -c version, which support output redirect.
// Thus, update accordingly.

/**
 * Is there a way to pass arbitrary commands? I don't think so, the redirect is handled by shel. So some shell library shall be used, e.g. bash, readline.
 * Another alternative: boost thread library.
 * @param [out] argv NULL terminated array
 */

static int split_cmd(const char *scon, char** &argv) {
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



/**
 * The cmd should be a single string contains NO white space.
 * Because the call is /bin/sh sh -c cmd
 * Any input redirect or arguments shall NOT working.
 */
std::string utils::exec(const char* cmd, int *status, double timeout_d) {
  int timeout = timeout_d * 1000000;
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(1);
  }
  pid_t pid;
  pid = fork();
  switch (pid) {
    case -1: perror("fork"); exit(1);
    case 0: {
      // child
      if (timeout>0) ualarm(timeout, 0);
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[0]);
      close(pipefd[1]);
      // execl("/bin/sh", "sh", "-c", cmd, (char *) NULL);
      // perror("execl");
      // exit(1);

      /**
       * Another alternative way to exec, which does not use the awkward shell to execute
       */
      
      // prepare the command
      char **argv = NULL;
      // the argv is malloc-ed, but anyway the process will exit, it will be released
      split_cmd(cmd, argv);
      // DEBUG
      // std::cerr << cmd  << "\n";
      execvp(argv[0], argv);
      perror(cmd);
      exit(1);
    }
    default: {
      // parent
      close(pipefd[1]);
      // fdopen(pipefd[0], "r");
      char buf[BUFSIZ];
      int nread;
      std::string result;
      while ((nread = read(pipefd[0], buf, sizeof(buf))) != 0) {
        if (nread == -1) {
          perror("read");
        } else {
          result.append(buf, nread);
        }
      }
      close(pipefd[0]);
      int _status;
      waitpid(pid, &_status, 0);
      // if (WIFEXITED(_status) && status != NULL) {
      //   *status = WEXITSTATUS(_status);
      // }
      if (status != NULL) {
        if (WIFEXITED(_status)) {
          *status = WEXITSTATUS(_status);
        } else if (WIFSIGNALED(*status)) {
          *status = TIMEOUT_RET_CODE;
          // result += "\nHELIUM_TEST_SIGNAL\n";
        }
      }
      return result;
    }
  }
}

/**
 * The cmd should be only the executable, followed by its arguments
 * The redirect will NOT be recognized
 * Thus exe < input.txt will be: "exe", argv[1]: "<", argv[2]: "input.txt"
 *
 * This will redirect stdout, so the stderr will not be viewable by the parent thread, thus not going to be the return output.
 */
std::string utils::exec_in(const char* cmd, const char* input, int *status, double timeout_d) {
  int timeout = timeout_d * 1000000;
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
  pid = fork();
  if (pid<0) {
    perror("fork");
    exit(1);
  }
  if (pid == 0) {
    // children
    if (timeout>0) ualarm(timeout, 0);
    if (dup2(pipein[0], 0) == -1 || dup2(pipeout[1], 1) == -1) {
      perror("dup error");
      exit(1);
    }
    int null_fd = open("/dev/null", O_RDONLY);
    dup2(null_fd, 2);
    close(pipein[0]);
    close(pipein[1]);
    close(pipeout[0]);
    close(pipeout[1]);
    // prepare the command
    char **argv = NULL;
    // the argv is malloc-ed, but anyway the process will exit, it will be released
    split_cmd(cmd, argv);
    execvp(argv[0], argv);
    perror(cmd);
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
  int _status;
  waitpid(pid, &_status, 0);
  if (status != NULL) {
    if (WIFEXITED(_status)) {
      *status = WEXITSTATUS(_status);
    } else if (WIFSIGNALED(*status)) {
      *status = TIMEOUT_RET_CODE;
      // result += "\nHELIUM_TEST_SIGNAL\n";
    }
  }
  return result;
}

TEST(thread_test_case, exec) {
  int status;
  status = 1;
  utils::exec("ls", &status);
  EXPECT_EQ(status, 0);
  // utils::exec("sss", &status);
  // EXPECT_EQ(status, 0);
  // std::string s = utils::exec("cat", "hello", &status);
  // EXPECT_EQ(status, 0);
  // EXPECT_EQ(s, "hello");
  // status = 0;
  // utils::exec("/tmp/helium-test-temp.YF3FoF/a.out", "65 G 8 Mg_Rv 76 1 ^ 3 Z 16", &status);
  // EXPECT_EQ(status, 1);
}

TEST(ThreadTestCase, TimeoutTest) {
  int status;
  status=0;
  // utils::exec_in("sleep 10", "test", &status, 1);
  // utils::exec_in("sleep 10", "this is input", &status, 1);
  // utils::exec_in("/Users/hebi/github/Helium/scratch/a.out", "3", &status, 1);
  // utils::exec("/Users/hebi/github/Helium/scratch/a.out", &status, 1);
  // utils::exec("/tmp/helium-test-tmp.2D6sfk/a.out", &status, 1);
  // std::cout << status  << "\n";
  utils::exec("sleep 10", &status, 0.5); // 100000 MICROseconds, 0.1s
  std::cout << status  << "\n";
}

/**
 * Use /bin/sh to execute the command, thus support output redirection.
 * BUT NOT INPUT REDIRECTION!
 */
std::string utils::exec_sh(const char* cmd, int *status, double timeout_d) {
  int timeout = timeout_d * 1000000;
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(1);
  }
  pid_t pid;
  pid = fork();
  switch (pid) {
    case -1: perror("fork"); exit(1);
    case 0: {
      // child
      if (timeout>0) ualarm(timeout, 0);
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[0]);
      close(pipefd[1]);
      /**
       * This command will just treat cmd as a whole in the argument argv[i], so no need to add quotes.
       * FIXME should I use sh or bash here?
       */
      execl("/bin/sh", "sh", "-c", cmd, (char *) NULL);
      perror("execl");
      exit(1);
    }
    default: {
      // parent
      close(pipefd[1]);
      // fdopen(pipefd[0], "r");
      char buf[BUFSIZ];
      int nread;
      std::string result;
      while ((nread = read(pipefd[0], buf, sizeof(buf))) != 0) {
        if (nread == -1) {
          perror("read");
        } else {
          result.append(buf, nread);
        }
      }
      close(pipefd[0]);
      int _status;
      waitpid(pid, &_status, 0);
      if (status != NULL) {
        if (WIFEXITED(_status)) {
          *status = WEXITSTATUS(_status);
        } else if (WIFSIGNALED(*status)) {
          *status = TIMEOUT_RET_CODE;
          // result += "\nHELIUM_TEST_SIGNAL\n";
        }
      }
      // if (WIFEXITED(_status) && status != NULL) {
      //   *status = WEXITSTATUS(_status);
      // }
      return result;
    }
  }

  
}



