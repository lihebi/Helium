#include "snippet/TypedefSnippet.hpp"
#include <regex>

// std::regex name_reg("struct\\s+(\\w+)");
static std::regex alias_reg("(\\w+)\\s*;\\s*");

TypedefSnippet::TypedefSnippet(const std::string& code, const std::string& filename, int line_number)
: m_code(code), m_type('t'), m_filename(filename), m_line_number(line_number) {
  std::smatch match;
  if (std::regex_search(code, match, alias_reg)) {
    m_name = match[1];
    m_keywords.insert(m_name);
  }
}
