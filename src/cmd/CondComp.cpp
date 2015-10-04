#include "cmd/CondComp.hpp"
#include "util/FileUtil.hpp"
#include "util/ThreadUtil.hpp"
#include "util/StringUtil.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cassert>
#include <stack>
#include "Config.hpp"

CondComp::CondComp(const std::string &folder) : m_folder(folder) {
  std::vector<std::string> extension {"c", "h"};
  FileUtil::GetFilesByExtension(m_folder, m_files, extension);
  getUsedMacros();
  getDefinedMacros();
  // m_defined_macros = m_macros;
  std::string s = Config::Instance()->GetCondCompMacros();
  if (!s.empty()) {
    std::string path = std::string(std::getenv("HELIUM_HOME")) + "/" + s;
    parseFile(path);
  }
  // print out
  std::cout << "Used Macros:" << std::endl;
  for (auto it=m_macros.begin();it!=m_macros.end();it++) {
    std::cout << "\t" << *it << std::endl;
  }
  std::cout << "Defined Macros:" << std::endl;
  for (auto it=m_defined_macros.begin();it!=m_defined_macros.end();it++) {
    std::cout << "\t" << *it << std::endl;
  }
}

void
CondComp::parseFile(const std::string& file) {
  std::cout << "[CondComp::parseFile]" << file << std::endl;
  std::ifstream is;
  is.open(file);
  std::string line;
  if (is.is_open()) {
    while (std::getline(is, line)) {
      StringUtil::trim(line);
      // only support // at the beginning
      if (line.find("//") == 0) continue;
      // now only support #undef
      if (line.find("#undef") == 0) {
        m_macros.insert(StringUtil::Split(line)[1]);
      }
    }
    is.close();
  }
}

std::string
get_check_code(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  s = s.substr(5);
  if (s.back() == 'h') {
    // HAVE_XXX_XXX_H
    replace(s.begin(), s.end(), '_', '/');
    s[s.rfind('/')] = '.';
    return "AC_CHECK_HEADERS([" + s + "])\n";
  } else {
    // HAVE_XXX
    return "AC_CHECK_FUNCS(" + s + ")\n";
  }
}

void
CondComp::getUsedMacros() {
  // run ifnames to get all HAVE_XXX and HAVE_XXX_H macros used
  // ls **/*.c can only be used in zsh
  // std::string cmd = "ifnames " + m_folder + "/**/*.[ch]";
  std::string cmd = "ifnames `find " + m_folder + " -name \"*.[ch]\"`";
  std::string output = ThreadUtil::Exec(cmd);
  std::vector<std::string> defines = StringUtil::Split(output, '\n');
  for (auto it=defines.begin();it!=defines.end();it++) {
    if (it->find("HAVE") == 0) {
      m_macros.insert(it->substr(0, it->find(' ')));
    }
  }
}

void
CondComp::getDefinedMacros() {
  // construct a automake file, test the result on current system
  std::string config_ac_code =
  "AC_INIT([helium], [0.1], [xxx@xxx.com])\n"
  // "AM_INIT_AUTOMAKE([foreign -Wall -Werror])\n"
  // "AC_PROG_CC\n"
  "AC_CONFIG_HEADERS([config.h])\n"
  ;
  for (auto it=m_macros.begin();it!=m_macros.end();it++) {
    config_ac_code += get_check_code(*it);
  }
  config_ac_code += "AC_OUTPUT\n";
  FileUtil::Write("/tmp/helium/configure.ac", config_ac_code);
  std::string cmd = "cd /tmp/helium && autoreconf && ./configure";
  std::cout << "[CondComp::Run] running: " << cmd << std::endl;
  if (ThreadUtil::ExecExit(cmd) == 0) {
    std::cout << "\033[32m" << "successfully executed" << "\033[0m" << std::endl;
    std::ifstream is;
    is.open("/tmp/helium/config.h");
    if (is.is_open()) {
      std::string line;
      while (getline(is, line)) {
        // std::cout << line << std::endl;
        if (line[0] == '#' && line.find("HAVE") == strlen("#define ")) {
          line = line.substr(line.find("HAVE"));
          line = line.substr(0, line.find(' '));
          m_defined_macros.insert(line);
        }
      }
      is.close();
    }
  }
}

/*
 * #elif HASH_BIG_ENDIAN == 1
 * #if 0
 * #if !defined(__GNUC__) || (__GNUC__ == 2 && __GNUC_MINOR__ < 96)
 */
std::vector<std::string>
get_tokens(const char *s_const) {
  char *s = strdup(s_const);
  std::vector<std::string> tokens;
  char *token = strtok(s, " ");
  while (token) {
    if (strcmp(token, "#if") == 0) {
      // tokens.push_back("#if");
    } else if (strcmp(token, "#ifdef") == 0) {
      // tokens.push_back("#ifdef");
    } else if (strcmp(token, "#ifndef") == 0) {
      // tokens.push_back("#ifndef");
      tokens.push_back("!");
    } else if (strcmp(token, "#elif") == 0) {
      // tokens.push_back("#elif");
    } else if (strcmp(token, "!") == 0) {
      tokens.push_back("!");
    } else if (strncmp(token, "defined", strlen("defined")) == 0) {
      // tokens.push_back("defined");
      if (strlen(token) > strlen("defined") + 1) {
        std::string tmp(token+strlen("defined("));
        if (tmp.back() == ')') tmp.pop_back();
        tokens.push_back(tmp);
      }
    } else if (strncmp(token, "!defined", strlen("!defined")) == 0) {
      tokens.push_back("!");
      // tokens.push_back("defined")
      if (strlen(token) > strlen("!defined") + 1) {
        std::string tmp(token+strlen("!defined("));
        if (tmp.back() == ')') tmp.pop_back();
        tokens.push_back(tmp);
      }
    } else if (strcmp(token, "&&") == 0) {
      // FIXME what if #if defined(xx)&&defined xx
      tokens.push_back("&&");
    } else if (strcmp(token, "||") == 0) {
      tokens.push_back("||");
    } else {
      // macros
      tokens.push_back(token);
    }
    token = strtok(NULL, " ");
  }
  free(s);
  return tokens;
}

int
CondComp::query(const std::string& id) {
  if (m_macros.find(id) == m_macros.end()) return 0;
  if (m_defined_macros.find(id) == m_defined_macros.end()) return -1;
  return 1;
}

/*
 * -1: delete
 *  0: ignore
 *  1: keep
 */
int
CondComp::getAction(const std::string& line) {
  // std::cout << "[CondComp::getAction] " << line << std::endl;
  // #if defined(HAVE_GETPAGESIZES) && defined(HAVE_MEMCNTL)
  // we do not handle #if (defined() && defined()), i.e. do not use () to change precedence
  bool positive = true; // the current ! ifn status
  bool is_and = true; // the current && or || status
  int result = 0;
  std::vector<std::string> tokens = get_tokens(line.c_str());
  for (auto it=tokens.begin();it!=tokens.end();it++) {
    if (*it == "!") {
      positive = !positive;
    } else if (*it == "&&") {
      positive = true;
      is_and = true;
    } else if (*it == "||") {
      positive = true;
      is_and = false;
    } else {
      // macro
      // std::cout << "\033[32m"<<"token: " << *it <<"\033[0m"<< std::endl;
      int tmp = query(*it);
      if (!positive) tmp = -tmp;
      if (tmp != 0) {
        if (result == 0) result = tmp;
        else {
          if (is_and) result = (result == 1 && tmp == 1) ? 1 : -1;
          else result = (result == 1 || tmp == 1) ? 1 : -1;
        }
      }
      // std::cout << "\033[33m" << "result: " << result << "\033[0m" << std::endl;
    }
  }
  return result;
}

bool
CondComp::process(std::vector<std::string>& lines) {
  // std::cout << "[CondComp::process]" << std::endl;
  std::string line;
  std::stack<int> action_stack = std::stack<int>();
  int level = 0;
  action_stack.push(0);
  bool changed = false;
  for (auto it=lines.begin();it!=lines.end();it++) {
    line = *it;
    // std::cout << *it << std::endl;
    StringUtil::trim(line);
    // the level is beyond stack. The only case is level goes up while action = -1
    if (level != action_stack.size()-1) {
      // std::cout << "\033[31m" << *it << "\033[0m" << std::endl;
      if (line.find("#if") == 0) level++;
      else if (line.find("#endif") == 0) level--;
      else {
        changed = true;
        lines.erase(it);
        it--;
      }
      continue;
    }
    // adjust level
    if (line.find("#if") == 0) level++;
    if (line.find("#endif") == 0) level--;

    // handle different cases
    if (line.find("#if") == 0) {
      // if current action is -1, then do not push, because everything should be deleted
      if (action_stack.top() != -1) {
        action_stack.push(getAction(line));
      }
    } else if (line.find("#else") == 0) {
      // reverse action
      int tmp = action_stack.top();
      tmp = -tmp;
      action_stack.pop();
      action_stack.push(tmp);
    } else if (line.find("#elif") == 0) {
      // FIXME change action
      action_stack.pop();
      action_stack.push(getAction(line));
    } else if (line.find("#endif") == 0) {
      // pop action
      action_stack.pop();
    } else {
      // delete the line
      if (action_stack.top() == -1) {
        // std::cout << "\033[31m" << *it << "\033[0m" << std::endl;
        changed = true;
        lines.erase(it);
        it--;
      }
    }
  }
  return changed;
  // std::cout << "stack size: " << action_stack.size() << std::endl;
}

void
CondComp::Run() {
  std::cout << "[CondComp::Run]" << std::endl;
  // use those result, modify all the files
  for (auto it=m_files.begin();it!=m_files.end();it++) {
    // read file
    std::string content = FileUtil::Read(*it);
    std::vector<std::string> lines = StringUtil::Split(content, '\n');
    // process lines
    if (process(lines)) {
      std::cout << "[CondComp::Run] " << *it << " changed" << std::endl;
    }
    // write file
    std::string code;
    for (auto it=lines.begin();it!=lines.end();it++) {
      code += *it + '\n';
    }
    FileUtil::Write(*it, code);
  }
}
