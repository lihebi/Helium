#ifndef ARGV_H
#define ARGV_H

#include "common.h"
#include "input_spec.h"

class ArgV {
public:
  static ArgV* Instance() {
    if (!m_instance) {
      m_instance = new ArgV();
    }
    return m_instance;
  }
  void clear() {
    m_argc = 0;
    m_argv.clear();
  }
  void SetOpt(std::string opt) {
    m_opt = opt;
    for (int i=opt.length()-1;i>=0;i--) {
      if (opt[i] == ':') {
        i--;
        m_str_opt.insert(opt[i]);
      } else {
        m_bool_opt.insert(opt[i]);
      }
    }
  }
  void ClearOpt() {
    m_opt = "";
  }
  InputSpec* GetArgVInput();
  InputSpec* GetArgCInput();
private:
  ArgV() {}
  ~ArgV() {}
  static ArgV *m_instance;
  void generate();
  int m_argc;
  std::vector<std::string> m_argv;
  std::string m_opt;
  std::set<char> m_bool_opt;
  std::set<char> m_str_opt;
};

#endif /* ARGV_H */
