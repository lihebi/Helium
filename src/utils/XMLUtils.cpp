#include "helium/utils/xml_utils.h"
#include "helium/utils/common.h"
#include "helium/utils/thread_utils.h"

namespace utils {

  /*******************************
   ** SrcmlUtil
   *******************************/

  /**
   * Query query on code.
   * return the first matching.
   */
  std::string query_xml_first(const std::string& xml_file, const std::string& query) {
    pugi::xml_document doc;
    file2xml(xml_file, doc);
    pugi::xml_node root_node = doc.document_element();
    pugi::xml_node node = root_node.select_node(query.c_str()).node();
    return node.child_value();
  }
  /**
   * Query "query" on "code".
   * Return all matching.
   * Will not use get_text_content, but use child_value() for a xml tag.
   * Only support tag value currently, not attribute value.
   */
  std::vector<std::string> query_xml(const std::string& xml_file, const std::string& query) {
    std::vector<std::string> result;
    pugi::xml_document doc;
    file2xml(xml_file, doc);
    pugi::xml_node root_node = doc.document_element();
    pugi::xpath_node_set nodes = root_node.select_nodes(query.c_str());
    for (auto it=nodes.begin();it!=nodes.end();it++) {
      pugi::xml_node node = it->node();
      result.push_back(node.child_value());
    }
    return result;
  }

  std::map<std::string, pugi::xml_document*> xml_docs;

  /**
   * Create doc, return the handler (pointer)
   * Don't free it outside.
   * All such files are maintained and looked up.
   * This function will cache the xml documents, thus is very cheap if you use it for the same file many times
   * However, it does assume storage.
   */
  pugi::xml_document* file2xml(const std::string &filename) {
    if (xml_docs.count(filename) == 1) return xml_docs[filename];
    std::string cmd;
    // cmd = "srcml --position -lC " + filename;
    // cmd = "helium-srcml --position " + filename;
    cmd = "srcml-client.py " + filename;
    // cmd = "srcml " + filename;
    std::string xml = exec(cmd.c_str(), NULL);
    pugi::xml_document *doc = new pugi::xml_document();
    doc->load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
    xml_docs[filename] = doc;
    return doc;
  }



  /**
   * convert a file into srcml output
   * @param[in] filename
   * @param[out] doc doc must be created by caller
   */
  void file2xml(const std::string &filename, pugi::xml_document& doc) {
    std::string cmd;
    // cmd = "srcml --position -lC " + filename;
    // cmd = "helium-srcml --position " + filename;
    cmd = "srcml-client.py " + filename;
    // cmd = "srcml " + filename;
    std::string xml = exec(cmd.c_str(), NULL);
    doc.load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  }
  // c code will be converted into xml, and loaded by doc
  /**
   * Convert a string of code into srcml output.
   * Only support C code for now.
   * @param[in] code
   * @param[out] doc
   */
  void string2xml(const std::string &code, pugi::xml_document& doc) {
    // std::string cmd = "srcml --position -lC";
    std::string cmd;
    // cmd = "helium-srcml --position -";
    cmd = "srcml-client.py -";
    // std::string cmd = "srcml -lC";
    std::string xml = exec_in(cmd.c_str(), code.c_str(), NULL);
    doc.load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  }

}
