#include "snippet/EnumSnippet.hpp"
#include <regex>
#include <iostream>
#include "util/DomUtil.hpp"
#include "util/SrcmlUtil.hpp"

static std::regex name_reg("enum\\s+(\\w+)");
static std::regex alias_reg("(\\w+)\\s*;\\s*");

EnumSnippet::EnumSnippet(const std::string& code, const std::string& filename, int line_number)
: m_code(code), m_type('g'), m_filename(filename), m_line_number(line_number) {
  std::smatch name_match;
  std::smatch alias_match;
  std::string tmp = code.substr(0, code.find('{'));
  std::regex_search(tmp, name_match, name_reg);
  if (!name_match.empty()) {
    m_name = name_match[1];
    m_keywords.insert(m_name);
    m_name = "enum "+m_name;
  }

  tmp = code.substr(code.rfind('}'));
  std::regex_search(tmp, alias_match, alias_reg);
  if (!alias_match.empty()) {
    m_alias = alias_match[1];
    m_keywords.insert(m_alias);
  }
  // TODO NOW Enum members should be in the keywords
  pugi::xml_document doc;
  SrcmlUtil::String2XML(m_code, doc);
  pugi::xml_node root_node = doc.document_element();
  pugi::xml_node enum_node = root_node.select_node("//enum").node();
  pugi::xpath_node_set name_nodes = enum_node.select_nodes("block/decl/name");
  for (int i=0;i<name_nodes.size();i++) {
    std::string s = DomUtil::GetTextContent(name_nodes[i].node());
    // std::cout << "\t" << s << std::endl;
    // Add enum member names into keywords
    m_keywords.insert(s);
  }
}
