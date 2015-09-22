#include "snippet/VariableSnippet.hpp"
#include <regex>

static std::regex name_reg("");

VariableSnippet::VariableSnippet(const std::string& code, const std::string& id, const std::string& filename, int line_number)
: m_code(code), m_name(id), m_type('v'), m_filename(filename), m_line_number(line_number) {
  m_keywords.insert(m_name);
}
