#include "snippet/VariableSnippet.hpp"
#include <regex>

static std::regex name_reg("");

VariableSnippet::VariableSnippet(const std::string& code, const std::string& id)
: m_code(code), m_name(id), m_type('v') {
  m_keywords.insert(m_name);
}
