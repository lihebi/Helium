#include "snippet/VariableSnippet.hpp"
#include <regex>
#include "util/FileUtil.hpp"

static std::regex name_reg("");

VariableSnippet::VariableSnippet(const std::string& code, const std::string& id, const std::string& filename, int line_number)
: m_code(code), m_name(id), m_type('v'), m_filename(filename), m_line_number(line_number) {
  m_keywords.insert(m_name);
}

VariableSnippet::VariableSnippet(const CtagsEntry& ce) {
  m_name = ce.GetName();
  m_type = 'v';
  m_filename = ce.GetSimpleFileName();
  m_line_number = ce.GetLineNumber();
  m_code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  m_keywords.insert(m_name);
}
