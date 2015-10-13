#include "snippet/StructureSnippet.hpp"
#include <boost/regex.hpp>
#include "util/FileUtil.hpp"
#include <iostream>

/*
FIXME
typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID Pointer;
	} DUMMYUNIONNAME;

	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;
*/

static boost::regex name_reg("struct\\s+(\\w+)");
static boost::regex alias_reg("(\\w+)\\s*;\\s*");

static void
get_keywords(
  const std::string& code,
  std::string& name, std::string& alias, std::set<std::string>& keywords
) {
  boost::smatch name_match;
  boost::smatch alias_match;
  std::string tmp = code.substr(0, code.find('{'));
  boost::regex_search(tmp, name_match, name_reg);
  if (!name_match.empty()) {
    name = name_match[1];
    keywords.insert(name);
  }

  tmp = code.substr(code.rfind('}'));
  boost::regex_search(tmp, alias_match, alias_reg);
  // boost::smatch keyword_match;
  // boost::regex_search(tmp, keyword_match, boost::regex("\\b(\\w+)\\b"))


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
