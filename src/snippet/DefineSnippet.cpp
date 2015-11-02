#include "snippet/DefineSnippet.hpp"
#include "util/FileUtil.hpp"
#include <boost/regex.hpp>

DefineSnippet::DefineSnippet(const CtagsEntry& ce) {
  m_type = 'd';
  m_filename = ce.GetSimpleFileName();
  m_line_number = ce.GetLineNumber();
  m_code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  m_loc = std::count(m_code.begin(), m_code.end(), '\n');
  m_name = ce.GetName();
  m_keywords.insert(m_name);
}
