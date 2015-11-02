#include "snippet/VariableSnippet.hpp"
#include "util/FileUtil.hpp"

VariableSnippet::VariableSnippet(const CtagsEntry& ce) {
  m_name = ce.GetName();
  m_type = 'v';
  m_filename = ce.GetSimpleFileName();
  m_line_number = ce.GetLineNumber();
  m_code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  m_loc = std::count(m_code.begin(), m_code.end(), '\n');
  m_keywords.insert(m_name);
}
