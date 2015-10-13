#include "snippet/UnionSnippet.hpp"
#include "util/FileUtil.hpp"
#include <boost/regex.hpp>

static boost::regex name_reg("union\\s+(\\w+)");
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
    name = "union "+name;
  }

  tmp = code.substr(code.rfind('}'));
  boost::regex_search(tmp, alias_match, alias_reg);
  if (!alias_match.empty()) {
    alias = alias_match[1];
    name += "alias "+alias;
    keywords.insert(alias);
  }
}
UnionSnippet::UnionSnippet(const std::string& code, const std::string& filename, int line_number)
: m_code(code), m_type('u'), m_filename(filename), m_line_number(line_number) {
  get_keywords(code, m_name, m_alias, m_keywords);
}

UnionSnippet::UnionSnippet(const CtagsEntry& ce) {
  m_type = 'u';
  m_filename = ce.GetSimpleFileName();
  m_line_number = ce.GetLineNumber();
  m_code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  get_keywords(m_code, m_name, m_alias, m_keywords);
}
