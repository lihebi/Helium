#include "snippet/DefineSnippet.hpp"
#include "util/FileUtil.hpp"
#include <regex>

// FIXME \\w+ will match the longest string?
std::regex define_name_reg("define\\s+(\\w+)");
DefineSnippet::DefineSnippet(const std::string& code, const std::string& id, const std::string& filename, int line_number)
: m_code(code), m_type('d'), m_filename(filename), m_line_number(line_number) {
  std::smatch match;
  // if (std::regex_search(code, match, define_name_reg)) {
  //   m_name = match[1];
  //   m_keywords.insert(m_name);
  // }
  m_name = id;
  m_keywords.insert(m_name);
}

DefineSnippet::DefineSnippet(const CtagsEntry& ce) {
  m_type = 'd';
  m_filename = ce.GetSimpleFileName();
  m_line_number = ce.GetLineNumber();
  m_code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  m_name = ce.GetName();
  m_keywords.insert(m_name);
}
