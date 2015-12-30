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

/*
 * TODO This function is too buggy.
 * The most common fault is libstdc++ string out-of-range
 * It may be caused by code is empty.
 * Or code is not a struct definitio: e.g. typedef struct conn * conn;
 */
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

void
StructureSnippet::getName(const CtagsEntry& ce) {
  if (ce.GetType() == 't') {
    std::string typeref = ce.GetTyperef();
    std::string name = typeref.substr(strlen("struct:"));
    if (name.substr(0, strlen("__anon")) == "__anon") {
      // anonymouse structure
      m_name = "";
    } else {
      // we already get the refer struct name from ctags, no need to parse the code!
      m_name = name;
    }
    m_alias = ce.GetName();
  } else {
    m_name = ce.GetName();
  }
}

StructureSnippet::StructureSnippet(const CtagsEntry& ce) {
  m_type = 's';
  m_filename = ce.GetSimpleFileName();
  m_line_number = ce.GetLineNumber();
  // m_code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  getName(ce);
  m_code = GetStructCode(ce.GetFileName(), m_line_number, m_name, m_alias);
  m_loc = std::count(m_code.begin(), m_code.end(), '\n');
  get_keywords(m_code, m_name, m_alias, m_keywords);
}
