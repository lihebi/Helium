#include "snippet/EnumSnippet.hpp"
#include <boost/regex.hpp>
#include <iostream>
#include "util/DomUtil.hpp"
#include "util/SrcmlUtil.hpp"
#include "util/FileUtil.hpp"
#include <Logger.hpp>

static boost::regex name_reg("enum\\s+(\\w+)");
static boost::regex alias_reg("(\\w+)\\s*;\\s*");

static void
get_keywords(
  const std::string& code,
  std::set<std::string>& keywords
) {
  Logger::Instance()->LogTraceV("[EnumSnippet::get_keywords]\n");
   // TODO NOW Enum members should be in the keywords
  pugi::xml_document doc;
  SrcmlUtil::String2XML(code, doc);
  pugi::xml_node root_node = doc.document_element();
  pugi::xml_node enum_node = root_node.select_node("//enum").node();
  pugi::xpath_node_set name_nodes = enum_node.select_nodes("block/decl/name");
  for (size_t i=0;i<name_nodes.size();i++) {
    std::string s = DomUtil::GetTextContent(name_nodes[i].node());
    // Add enum member names into keywords
    if (!s.empty()) keywords.insert(s);
  }
}

void
EnumSnippet::getName(const CtagsEntry& ce) {
  // FIXME buggy
  if (ce.GetType() == 'g') {
    m_name = ce.GetName();
  } else if (ce.GetType() == 'e') {
    std::string ref = ce.GetTyperef();
    // FIXME the ref can be enum:VideoState::ShowMode
    // Is it possible that it does not start with enum: ?
    m_name = ref.substr(ref.rfind(':')+1); // the last part of ref
  } else if (ce.GetType() == 't') {
    std::string typeref = ce.GetTyperef();
    std::string name = typeref.substr(strlen("enum:"));
    if (name.substr(0, strlen("__anon")) == "__anon") {
      // anonymouse structure
      m_name = "";
    } else {
      // we already get the refer struct name from ctags, no need to parse the code!
      m_name = name;
    }
    m_alias = ce.GetName();
  }
}


EnumSnippet::EnumSnippet(const CtagsEntry& ce) {
  Logger::Instance()->LogTraceV("[EnumSnippet::EnumSnippet] " + ce.GetName() + "\n");
  m_type = 'g';
  m_filename = ce.GetSimpleFileName();
  m_line_number = ce.GetLineNumber();
  // m_code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  // m_code = GetEnumCode(ce.GetFileName(), ce.GetLineNumber(), ce.GetName());
  getName(ce);  
  m_code = GetEnumCode(ce.GetFileName(), ce.GetLineNumber());
  m_loc = std::count(m_code.begin(), m_code.end(), '\n');
  get_keywords(m_code, m_keywords);
  if (!m_name.empty()) m_keywords.insert(m_name);
  if (!m_alias.empty()) m_keywords.insert(m_alias);
}
