#include "snippet/Snippet.hpp"
#include "util/SrcmlUtil.hpp"
#include <iostream>
#include <cstdlib>
#include <cassert>
#include "util/DomUtil.hpp"

/*
 * use depth-first-search for the first pos:line attribute
 * return -1 if no pos:line attr found
 */
int
get_element_line(pugi::xml_node node) {
  // check if pos:line is enabled on this xml
  pugi::xml_node root = node.root();
  if (!root.child("unit").attribute("xmlns:pos")) {
    std::cerr<<"position is not enabled in srcml"<<std::endl;
    exit(1);
    return -1;
  }
  // the node itself has pos:line attr, just use it
  if (node.attribute("pos:line")) {
    return atoi(node.attribute("pos:line").value());
  } else {
    pugi::xml_node n = node.select_node("//*[@pos:line]").node();
    if (n) {
      return atoi(n.attribute("pos:line").value());
    }
  }
  return -1;
}

/*
 * The last pos:line in the current node element
 * Useful to track the range of the current AST node.
 */
int
get_element_last_line(pugi::xml_node node) {
  pugi::xml_node root = node.root();
  if (!root.child("unit").attribute("xmlns:pos")) {
    std::cerr<<"position is not enabled in srcml"<<std::endl;
    exit(1);
    return -1;
  }
  pugi::xml_node n = node.select_node("(//*[@pos:line])[last()]").node();
  
  // pugi::xpath_node_set nodes = node.select_nodes("//*[@pos:line]");
  // pugi::xml_node last_node = nodes[nodes.size()-1].node();
  // return atoi(last_node.attribute("pos:line").value());
  
  if (n) {
    return atoi(n.attribute("pos:line").value());
  } else if (node.attribute("pos:line")) {
    return atoi(node.attribute("pos:line").value());
  }
  return -1;
}

/*
 * Get function code based on:
 * - line number
 * - function name (optional)
 * Use function name whenever possible (DEBUG may need to verify by line)
 * if not, use line number
 */
std::string
Snippet::GetFunctionCode(std::string filename, int line, std::string function_name) {
  assert(line>=0);
  assert(!filename.empty());
  // parse the file to xml tree
  pugi::xml_document doc;
  SrcmlUtil::File2XML(filename, doc);
  pugi::xml_node root = doc.document_element();
  if (!function_name.empty()) {
    // match by function name (for C, only one will match)
    std::string query = "//function[name='" + function_name + "']";
    pugi::xml_node function_node = root.select_node(query.c_str()).node();
    if (function_node) return DomUtil::GetTextContent(function_node);
    else return "";
  } else {
    // match by line
    pugi::xpath_node_set functions = root.select_nodes("//function");
    for (auto it=functions.begin();it!=functions.end();it++) {
      pugi::xml_node node = it->node();
      int _line = get_element_line(node);
      // FIXME line may not match which ctags gives me
      if (line == _line) return DomUtil::GetTextContent(node);
    }
    return "";
  }
}

std::string
Snippet::GetEnumCode(std::string filename, int line, std::string enum_name) {
  assert(line>=0);
  pugi::xml_document doc;
  SrcmlUtil::File2XML(filename, doc);
  pugi::xml_node root = doc.document_element();
  if (!enum_name.empty()) {
    // match by enum name
    std::string query = "//enum[name='" + enum_name + "']";
    pugi::xml_node enum_node = root.select_node(query.c_str()).node();
    if (enum_node) return DomUtil::GetTextContent(enum_node);
    else return "";
  } else {
    // match by line number
    pugi::xpath_node_set enums = root.select_nodes("//enum");
    for (auto it=enums.begin();it!=enums.end();it++) {
      pugi::xml_node node = it->node();
      int first_line = get_element_line(node);
      int last_line = get_element_last_line(node);
      if (first_line <= line && last_line >=line) {
        return DomUtil::GetTextContent(node);
      }
    }
    return "";
  }
}

std::string
Snippet::GetDefineCode(std::string filename, int line) {
  pugi::xml_document doc;
  SrcmlUtil::File2XML(filename, doc);
  pugi::xml_node root = doc.document_element();
  pugi::xpath_node_set defines = root.select_nodes("//cpp:define");
  for (auto it=defines.begin();it!=defines.end();it++) {
    pugi::xml_node node = it->node();
    int first_line = get_element_line(node);
    int last_line = get_element_last_line(node);
    if (first_line <= line && last_line >= line) {
      return DomUtil::GetTextContent(node);
    }
  }
  return "";
}



std::string
Snippet::GetTypedefCode(std::string filename, int line, std::string alias) {
  pugi::xml_document doc;
  SrcmlUtil::File2XML(filename, doc);
  pugi::xml_node root = doc.document_element();
  std::string query = "//typedef[name='" + alias + "']";
  pugi::xml_node typedef_node = root.select_node(query.c_str()).node();
  return DomUtil::GetTextContent(typedef_node);
}

/*
 * As a template for: struct, union
 * tag will be "struct" and "union"
 * if has alias, means it is actually a "typedef struct"
 * then name is not used.
 * if alias is empty, then search for "struct" whose name is "name"
 */
std::string
Snippet::GetCode(std::string filename, int line, std::string name, std::string alias, std::string tag) {
  pugi::xml_document doc;
  SrcmlUtil::File2XML(filename, doc);
  pugi::xml_node root = doc.document_element();
  // get code
  if (alias.empty()) {
    std::string query = "//"+tag+"[name='" + name + "']";
    pugi::xml_node struct_node = root.select_node(query.c_str()).node();
    return DomUtil::GetTextContent(struct_node);
  } else {
    std::string query = "//typedef[name='" + alias + "']";
    pugi::xml_node typedef_node = root.select_node(query.c_str()).node();
    return DomUtil::GetTextContent(typedef_node);
  }
  return "";
}
std::string
Snippet::GetStructCode(std::string filename, int line, std::string name, std::string alias) {
  return GetCode(filename, line, name, alias, "struct");
}

std::string
Snippet::GetUnionCode(std::string filename, int line, std::string name, std::string alias) {
  return GetCode(filename, line, name, alias, "union");
}
std::string
Snippet::GetVariableCode(std::string filename, int line, std::string name) {
  pugi::xml_document doc;
  SrcmlUtil::File2XML(filename, doc);
  pugi::xml_node root = doc.document_element();
  pugi::xpath_node_set decl_stmt_nodes = root.select_nodes("//decl_stmt");
  for (auto it=decl_stmt_nodes.begin();it!=decl_stmt_nodes.end();it++) {
    pugi::xml_node node = (*it).node();
    int first_line = get_element_line(node);
    int last_line = get_element_last_line(node);
    if (first_line <= line && last_line >= line) {
      return DomUtil::GetTextContent(node);
    }
  }
  return "";
}
 
