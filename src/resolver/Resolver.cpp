#include "resolver/Resolver.hpp"
#include "snippet/SnippetRegistry.hpp"
#include "resolver/Ctags.hpp"
#include "util/FileUtil.hpp"

#include <iostream>

const std::set<std::string> c_common_keyword = {
// def, head
"define", "undef", "ifdef", "ifndef",
"main",  "include",
// control branch keyword
"if", "else", "switch", "case", "default", "for", "do", "while", "break", "goto", "break", "continue",
// type
"bool", "true", "false"
// storage class specifier
"auto", "register", "static", "extern", "typedef",
// type specifier
"void", "char", "short", "int", "long", "float", "double", "signed", "unsigned",
"struct", "enum",
// type qualifier
"const", "volatile",
// undefined
"sizeof", "return", "asm"
};

static std::regex id_reg("\\b[_a-zA-Z][_a-zA-Z0-9]*\\b");

std::set<std::string>
Resolver::ExtractToResolve(const std::string& code) {
  std::smatch match;
  std::string tmp = "shitwhile while while";
  // std::regex_search(tmp, match, id);
  std::sregex_iterator begin(code.begin(), code.end(), id_reg);
  std::sregex_iterator end = std::sregex_iterator();
  std::set<std::string> ss;
  for (std::sregex_iterator it=begin;it!=end;it++) {
    std::string tmp = (*it).str();
    if (c_common_keyword.find(tmp) == c_common_keyword.end()) {
      ss.insert(tmp);
    }
  }
  return ss;
}
