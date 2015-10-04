#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <pugixml/pugixml.hpp>

#include "cmd/Splitter.hpp"
#include "util/FileUtil.hpp"
#include "util/StringUtil.hpp"
#include "util/SrcmlUtil.hpp"
#include "util/DomUtil.hpp"

namespace fs=boost::filesystem;

Splitter::Splitter(const std::string &folder) : m_folder(folder) {
  std::cout<<"[Splitter][Constructor]"<<std::endl;
}

Splitter::~Splitter() {}

std::string get_type(const std::string& s) {
  std::string part = s.substr(0, s.find(','));
  // remove array
  part = part.substr(0, part.find('['));
  // remove init
  part = part.substr(0, part.find('='));
  StringUtil::trim(part);
  part = part.substr(0, part.rfind(' '));
  while (part.back() == '*' || part.back() == ' ' || part.back() == '&') {
    part.pop_back();
  }
  return part;
}
void split(std::string& content) {
  std::string type = get_type(content);
  std::vector<std::string> vs;
  StringUtil::Split(content, ',', vs);
  content = vs[0]+";\n";
  for (auto it=vs.begin()+1;it!=vs.end();it++) {
    content += type + " " + *it + ";\n";
  }
  content.pop_back();
  content.pop_back();
}

void Splitter::Run() {
  std::cout<<"[Splitter][Run]"<<std::endl;
  FileUtil::GetFilesByExtension(m_folder, m_files, "c");
  for (auto it=m_files.begin();it!=m_files.end();it++) {
    std::cout<<"[Splitter][Run][Debug]"<<*it<<std::endl;
    pugi::xml_document doc;
    SrcmlUtil::File2XML(*it, doc);
    pugi::xml_node root = doc.document_element();
    pugi::xpath_node_set decl_stmts = root.select_nodes("//decl_stmt");
    int change_count = 0;
    for (int i=0;i<decl_stmts.size();i++) {
      pugi::xml_node decl_stmt_node = decl_stmts[i].node();
      pugi::xpath_node_set decls = decl_stmt_node.select_nodes("./decl");
      if (decls.size()>1) {
        // needs split
        change_count++;
        std::string content = DomUtil::GetTextContent(decl_stmt_node);

        split(content);
        // std::cout<<"[Splitter][FROM]\033[31m"<<DomUtil::GetTextContent(decl_stmt_node)<<"\033[0m"<<std::endl;
        // std::cout<<"[Splitter][To]\033[32m"<<content<<"\033[0m"<<std::endl;
        pugi::xml_node parent = decl_stmt_node.parent();
        pugi::xml_node new_node = parent.insert_child_after("decl_stmt", decl_stmt_node);
        new_node.append_child(pugi::node_pcdata).set_value(content.c_str());
        parent.remove_child(decl_stmt_node);
      }
    }
    // write back
    if (change_count>0) {
      std::cout<<"[Splitter][Run]Made "<<change_count<<" splits"<<std::endl;
      std::string content = DomUtil::GetTextContent(root);
      std::ofstream of;
      of.open(*it);
      if (of.is_open()) {
        of<<content;
        of.close();
      }
    }
  }
}
