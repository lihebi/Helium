#ifndef __RESOLVER_HPP__
#define __RESOLVER_HPP__

#include <string>
#include <vector>
#include <set>
#include <regex>
#include "snippet/Snippet.hpp"

class ToResolve {
public:
  ToResolve(const std::string& name, const std::string& type)
  : m_name(name), m_type(type) {}
  ~ToResolve() {}
  std::string GetName() {return m_name;}
  std::string GetType() {return m_type;}
private:
  std::string m_name;
  std::string m_type;
};

const std::set<std::string> c_common_keyword = {
"define", "undef", "ifdef", "ifndef",
"main", "while", "include", "if", "else",
"static", "const", "volatile"
};

class Resolver {
public:
  Resolver(const std::string& code)
  : m_code(code), m_regex("\\b[_a-zA-Z][_a-zA-Z0-9]*\\b") {}
  ~Resolver() {}
  void Resolve();
  std::set<Snippet*> GetSnippets() {
    return m_snippets;
  }
  bool resolveLocal(std::string name);
  bool resolveSystem(std::string name);
private:
  void extractToResolve(const std::string& code);
  std::string m_code;
  // the pointer to Snippet, stored in snippet registery
  std::set<std::string> m_resolved;
  std::set<std::string> m_to_resolve;
  std::set<std::string> m_unresolved;
  std::set<Snippet*> m_snippets;
  std::regex m_regex;
};

#endif
