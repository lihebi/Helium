#include "snippet/EnumSnippet.hpp"
#include <boost/regex.hpp>
#include <iostream>
#include "util/DomUtil.hpp"
#include "util/SrcmlUtil.hpp"
#include "util/FileUtil.hpp"

static boost::regex name_reg("enum\\s+(\\w+)");
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
    name = "enum "+name;
  }

  tmp = code.substr(code.rfind('}'));
  boost::regex_search(tmp, alias_match, alias_reg);
  if (!alias_match.empty()) {
    alias = alias_match[1];
    keywords.insert(alias);
  }
  // TODO NOW Enum members should be in the keywords
  pugi::xml_document doc;
  SrcmlUtil::String2XML(code, doc);
  pugi::xml_node root_node = doc.document_element();
  pugi::xml_node enum_node = root_node.select_node("//enum").node();
  pugi::xpath_node_set name_nodes = enum_node.select_nodes("block/decl/name");
  for (size_t i=0;i<name_nodes.size();i++) {
    std::string s = DomUtil::GetTextContent(name_nodes[i].node());
    // std::cout << "\t" << s << std::endl;
    // Add enum member names into keywords
    keywords.insert(s);
  }
}

EnumSnippet::EnumSnippet(const CtagsEntry& ce) {
  m_type = 'g';
  m_filename = ce.GetSimpleFileName();
  m_line_number = ce.GetLineNumber();
  m_code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  m_loc = std::count(m_code.begin(), m_code.end(), '\n');
  get_keywords(m_code, m_name, m_alias, m_keywords);
}
