#ifndef ARGV_H
#define ARGV_H

#include "helium/utils/common.h"
#include "input_spec.h"

class ArgV {
public:
  static ArgV* Instance() {
    if (!m_instance) {
      m_instance = new ArgV();
    }
    return m_instance;
  }
  void clear();
  void SetOpt(std::string opt);
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
  // int m_argc;
  // std::vector<std::string> m_argv;
  

  std::string m_opt;
  std::set<char> m_bool_opt;
  std::set<char> m_str_opt;

  std::string m_argv0;
  std::vector<char> m_bool_input;
  std::map<char, InputSpec*> m_str_input;
  std::vector<InputSpec*> m_propositional_input;
};

#endif /* ARGV_H */
