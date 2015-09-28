#include "snippet/FunctionSnippet.hpp"
#include <regex>
#include "util/StringUtil.hpp"
#include <iostream>

FunctionSnippet::FunctionSnippet(const std::string& code, const std::string& id, const std::string& filename, int line_number)
: m_code(code), m_type('f'), m_filename(filename), m_line_number(line_number) {
  // std::cout << "[FunctionSnippet::FunctionSnippet]" << std::endl;
  // std::cout << code << std::endl;
  std::string head = code.substr(0, code.find('('));
  // StringUtil::trim(head);
  std::vector<std::string> vs = StringUtil::Split(head);
  // m_name = vs[vs.size()-1];
  m_name = id;
  m_keywords.insert(m_name);
}

std::string
FunctionSnippet::GetDecl() {
  std::string decl = m_code.substr(0, m_code.find('{')) + ";";
  if (std::count(decl.begin(), decl.end(), ';') > 1) return "";
  else return decl;
}
