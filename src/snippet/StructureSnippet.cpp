#include "snippet/StructureSnippet.hpp"
#include <regex>
#include "util/FileUtil.hpp"
#include <iostream>

static std::regex name_reg("struct\\s+(\\w+)");
static std::regex alias_reg("(\\w+)\\s*;\\s*");

static void
get_keywords(
  const std::string& code,
  std::string& name, std::string& alias, std::set<std::string>& keywords
) {
  std::smatch name_match;
  std::smatch alias_match;
  std::string tmp = code.substr(0, code.find('{'));
  std::regex_search(tmp, name_match, name_reg);
  if (!name_match.empty()) {
    name = name_match[1];
    keywords.insert(name);
  }

  tmp = code.substr(code.rfind('}'));
  std::regex_search(tmp, alias_match, alias_reg);

  if (!alias_match.empty()) {
    alias = alias_match[1];
    keywords.insert(alias);
  }
}

StructureSnippet::StructureSnippet(const std::string& code, const std::string& filename, int line_number)
: m_code(code), m_type('s'), m_filename(filename), m_line_number(line_number) {
  // std::cout << "[StructureSnippet::StructureSnippet]" << std::endl;
  // std::cout << code << std::endl;
  get_keywords(code, m_name, m_alias, m_keywords);
}

StructureSnippet::StructureSnippet(const CtagsEntry& ce) {
  m_type = 's';
  m_filename = ce.GetSimpleFileName();
  m_line_number = ce.GetLineNumber();
  m_code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  get_keywords(m_code, m_name, m_alias, m_keywords);
  // print();
}

void
StructureSnippet::print() {
  std::cout << "[StructureSnippet::print]" << std::endl;
  std::cout << "\tname: " << m_name << std::endl;
  std::cout << "\talias: " << m_alias << std::endl;
  std::cout << "\tfilename: " << m_filename << std::endl;
  std::cout << "\tline number: " << m_line_number << std::endl;
}
