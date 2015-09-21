#include "snippet/DefineSnippet.hpp"
#include <regex>

// FIXME \\w+ will match the longest string?
std::regex define_name_reg("define\\s+(\\w+)");
DefineSnippet::DefineSnippet(const std::string& code)
: m_code(code), m_type('d') {
  std::smatch match;
  if (std::regex_search(code, match, define_name_reg)) {
    m_name = match[1];
    m_keywords.insert(m_name);
  }
}
