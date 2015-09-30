#include "snippet/TypedefSnippet.hpp"
#include <regex>
#include <iostream>
#include "util/FileUtil.hpp"

// std::regex name_reg("struct\\s+(\\w+)");
static std::regex alias_reg("(\\w+)\\s*;\\s*");

TypedefSnippet::TypedefSnippet(const std::string& code, const std::string& id, const std::string& filename, int line_number)
: m_code(code), m_type('t'), m_filename(filename), m_line_number(line_number) {
  // std::cout << "[TypedefSnippet::TypedefSnippet]" << std::endl;
  // std::cout << code << std::endl;
  m_name = id;
  m_keywords.insert(m_name);
}

TypedefSnippet::TypedefSnippet(const CtagsEntry& ce) {
  m_type = 't';
  m_filename = ce.GetSimpleFileName();
  m_line_number = ce.GetLineNumber();
  m_code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  m_name = ce.GetName();
  m_keywords.insert(m_name);
}
