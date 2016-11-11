#include "xml_doc_reader.h"
#include "utils/utils.h"
#include "xmlnode_helper.h"
#include <iostream>
#include <boost/filesystem.hpp>

#include "resolver/snippet_db.h"
namespace fs = boost::filesystem;
XMLDocReader *XMLDocReader::m_instance = NULL;

XMLDoc *XMLDocReader::ReadFile(std::string filename) {
  if (!fs::exists(filename)) {
    std::cerr << "EE: file does not exist " << filename << "\n";
    return NULL;
  }
  filename = fs::canonical(fs::path(filename)).string();
  if (m_docs.count(filename) == 1) return m_docs[filename];
  std::string cmd;
  cmd = "srcml --position -lC " + filename;
  // cmd = "srcml " + filename;
  std::string xml = utils::exec(cmd.c_str(), NULL);
  pugi::xml_document *doc = new pugi::xml_document();
  doc->load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  m_docs[filename] = doc;
  return doc;
}
/**
 * This will always create a new copy of XML document.
 * Be careful, do not use this in the framework.
 * Instead, use the file version, thus the cache can be used.
 * This should only be used in test code, or create SrcML for small code.
 */
XMLDoc* XMLDocReader::ReadString(const std::string &code) {
  std::cerr << "warning: [XMLDocReader::ReadString] create from string" << "\n";
  std::string cmd = "srcml --position -lC";
  // std::string cmd = "srcml -lC";
  std::string xml = utils::exec_in(cmd.c_str(), code.c_str(), NULL);
  pugi::xml_document *doc = new pugi::xml_document();
  doc->load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  m_string_docs.push_back(doc);
  return doc;
}

/**
 * Create the XMLDoc for a snippet in database.
 */
XMLDoc* XMLDocReader::ReadSnippet(int id) {
  if (m_snippet_docs.count(id) == 1) return m_snippet_docs[id];
  std::string code = SnippetDB::Instance()->GetCode(id);
  XMLDoc *doc = CreateDocFromString(code);
  m_snippet_docs[id] = doc;
  return doc;
}

XMLDoc* XMLDocReader::CreateDocFromString(const std::string &code, std::string filename) {
  std::string cmd = "srcml --position -lC";
  if (!filename.empty()) {
    cmd += " -f " + filename;
  }
  // std::string cmd = "srcml -lC";
  std::string xml = utils::exec_in(cmd.c_str(), code.c_str(), NULL);
  pugi::xml_document *doc = new pugi::xml_document();
  doc->load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  return doc;
}

XMLDoc* XMLDocReader::CreateDocFromFile(std::string filename) {
  std::string cmd;
  cmd = "srcml --position -lC " + filename;
  std::string xml = utils::exec(cmd.c_str(), NULL);
  pugi::xml_document *doc = new pugi::xml_document();
  doc->load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  return doc;
}

/**
 * Query query on code.
 * return the first matching.
 */
std::string XMLDocReader::QueryCodeFirst(const std::string &code, std::string query) {
  pugi::xml_document *doc = XMLDocReader::CreateDocFromString(code);
  pugi::xml_node root_node = doc->document_element();
  pugi::xml_node node = root_node.select_node(query.c_str()).node();
  std::string ret = node.child_value();
  delete doc;
  return ret;
}

/**
 * Return the string instead of child_value
 */
std::string XMLDocReader::QueryCodeFirstDeep(const std::string &code, std::string query) {
  pugi::xml_document *doc = XMLDocReader::CreateDocFromString(code);
  pugi::xml_node root_node = doc->document_element();
  pugi::xml_node node = root_node.select_node(query.c_str()).node();
  std::string ret = get_text(node);
  // std::string ret = node.child_value();
  delete doc;
  return ret;
}

/**
 * Query if the xml has the node
 */
bool XMLDocReader::QueryCodeHas(const std::string &code, std::string query) {
  pugi::xml_document *doc = XMLDocReader::CreateDocFromString(code);
  pugi::xml_node root_node = doc->document_element();
  pugi::xml_node node = root_node.select_node(query.c_str()).node();
  if (node) return true;
  else return false;
}


/**
 * Query "query" on "code".
 * Return all matching.
 * Will not use get_text_content, but use child_value() for a xml tag.
 * Only support tag value currently, not attribute value.
 */
std::vector<std::string> XMLDocReader::QueryCode(const std::string &code, std::string query) {
  std::vector<std::string> ret;
  pugi::xml_document *doc = XMLDocReader::CreateDocFromString(code);
  pugi::xml_node root_node = doc->document_element();
  pugi::xpath_node_set nodes = root_node.select_nodes(query.c_str());
  for (auto it=nodes.begin();it!=nodes.end();it++) {
    pugi::xml_node node = it->node();
    ret.push_back(node.child_value());
  }
  delete doc;
  return ret;
}
