#include "snippet/FunctionSnippet.hpp"
#include <regex>
#include "util/StringUtil.hpp"

FunctionSnippet::FunctionSnippet(const std::string& code)
: m_code(code), m_type('f') {
  std::string head = code.substr(0, code.find('('));
  // StringUtil::trim(head);
  std::vector<std::string> vs = StringUtil::Split(head);
  m_name = vs[vs.size()-1];
  m_keywords.insert(m_name);
}
