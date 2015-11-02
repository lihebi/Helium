#include "snippet/FunctionSnippet.hpp"
#include <regex>
#include "util/StringUtil.hpp"
#include "util/FileUtil.hpp"
#include <iostream>

FunctionSnippet::FunctionSnippet(const CtagsEntry& ce) {
  m_type = 'f';
  m_filename = ce.GetSimpleFileName();
  m_line_number = ce.GetLineNumber();
  m_code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  m_loc = std::count(m_code.begin(), m_code.end(), '\n');
  m_name = ce.GetName();
  m_keywords.insert(m_name);
}

std::string
FunctionSnippet::GetDecl() {
  std::string decl = m_code.substr(0, m_code.find('{')) + ";";
  if (std::count(decl.begin(), decl.end(), ';') > 1) return "";
  else return decl;
}
