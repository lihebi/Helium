#include "snippet/TypedefSnippet.hpp"
#include <regex>
#include <iostream>

// std::regex name_reg("struct\\s+(\\w+)");
static std::regex alias_reg("(\\w+)\\s*;\\s*");

TypedefSnippet::TypedefSnippet(const std::string& code, const std::string& id, const std::string& filename, int line_number)
: m_code(code), m_type('t'), m_filename(filename), m_line_number(line_number) {
  // std::cout << "[TypedefSnippet::TypedefSnippet]" << std::endl;
  // std::cout << code << std::endl;
  std::smatch match;
  m_name = id;
  m_keywords.insert(m_name);
  // if (std::regex_search(code, match, alias_reg)) {
  //   m_name = match[1];
  //   m_keywords.insert(m_name);
  // }
}
