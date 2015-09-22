#include "snippet/StructureSnippet.hpp"
#include <regex>
#include <iostream>

static std::regex name_reg("struct\\s+(\\w+)");
static std::regex alias_reg("(\\w+)\\s*;\\s*");

StructureSnippet::StructureSnippet(const std::string& code, const std::string& filename, int line_number)
: m_code(code), m_type('s'), m_filename(filename), m_line_number(line_number) {
  std::smatch name_match;
  std::smatch alias_match;
  std::string tmp = code.substr(0, code.find('{'));
  std::regex_search(tmp, name_match, name_reg);
  if (!name_match.empty()) {
    m_name = name_match[1];
    m_keywords.insert(m_name);
  }

  tmp = code.substr(code.rfind('}'));
  std::regex_search(tmp, alias_match, alias_reg);

  if (!alias_match.empty()) {
    m_alias = alias_match[1];
    m_keywords.insert(m_alias);
  }
}
