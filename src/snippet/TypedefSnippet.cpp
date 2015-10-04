#include "snippet/TypedefSnippet.hpp"
#include <regex>
#include <iostream>
#include "util/FileUtil.hpp"
#include <pugixml/pugixml.hpp>
#include "util/SrcmlUtil.hpp"
#include "util/DomUtil.hpp"

// std::regex name_reg("struct\\s+(\\w+)");
static std::regex alias_reg("(\\w+)\\s*;\\s*");

TypedefSnippet::TypedefSnippet(const std::string& code, const std::string& id, const std::string& filename, int line_number)
: m_code(code), m_type('t'), m_filename(filename), m_line_number(line_number) {
  // std::cout << "[TypedefSnippet::TypedefSnippet]" << std::endl;
  // std::cout << code << std::endl;
  m_name = id;
  m_keywords.insert(m_name);
}

void
TypedefSnippet::semanticParse() {
  // fill m_from, m_to, m_typedef_type
  m_from = m_name;
  pugi::xml_document doc;
  SrcmlUtil::String2XML(m_code, doc);
  pugi::xml_node root = doc.document_element();
  pugi::xml_node typedef_node = root.select_node("//typedef").node();
  if (typedef_node) {
    pugi::xml_node type_node = typedef_node.child("type");
    // pugi::xml_node name_node = typedef_node.child("name");
    pugi::xml_node function_decl_node = typedef_node.child("function_decl");
    if (type_node) {
      m_typedef_type = TYPEDEF_TYPE;
      m_to = DomUtil::GetTextContent(type_node);
    } else if (function_decl_node) {
      m_typedef_type = TYPEDEF_FUNC_POINTER;
      // function_decl_node.remove_child("name");
      // TODO init function pointer?
    }
  }
}

TypedefSnippet::TypedefSnippet(const CtagsEntry& ce) {
  m_type = 't';
  m_filename = ce.GetSimpleFileName();
  m_line_number = ce.GetLineNumber();
  m_code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  m_name = ce.GetName();
  semanticParse();
  m_keywords.insert(m_name);
}
