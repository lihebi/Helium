#include "snippet/UnionSnippet.hpp"
#include <regex>

static std::regex name_reg("union\\s+(\\w+)");
static std::regex alias_reg("(\\w+)\\s*;\\s*");

UnionSnippet::UnionSnippet(const std::string& code, const std::string& filename, int line_number)
: m_code(code), m_type('u'), m_filename(filename), m_line_number(line_number) {
  std::smatch name_match;
  std::smatch alias_match;
  std::string tmp = code.substr(0, code.find('{'));
  std::regex_search(tmp, name_match, name_reg);
  if (!name_match.empty()) {
    m_name = name_match[1];
    m_name += "union "+m_name;
    m_keywords.insert(m_name);
  }

  tmp = code.substr(code.rfind('}'));
  std::regex_search(tmp, alias_match, alias_reg);
  if (!alias_match.empty()) {
    m_alias = alias_match[1];
    m_name += "alias "+m_alias;
    m_keywords.insert(m_alias);
  }
}
