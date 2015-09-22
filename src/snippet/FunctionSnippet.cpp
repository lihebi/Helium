#include "snippet/FunctionSnippet.hpp"
#include <regex>
#include "util/StringUtil.hpp"

FunctionSnippet::FunctionSnippet(const std::string& code, const std::string& filename, int line_number)
: m_code(code), m_type('f'), m_filename(filename), m_line_number(line_number) {
  std::string head = code.substr(0, code.find('('));
  // StringUtil::trim(head);
  std::vector<std::string> vs = StringUtil::Split(head);
  m_name = vs[vs.size()-1];
  m_keywords.insert(m_name);
}
