#include "cmd/CommentRemover.hpp"
#include "util/FileUtil.hpp"
#include "util/DomUtil.hpp"
#include "util/SrcmlUtil.hpp"
#include <pugixml.hpp>
#include <fstream>
#include <iostream>

CommentRemover::CommentRemover(const std::string &folder) : m_folder(folder) {
  std::cout<<"[CommentRemover][Constructor]"<<std::endl;
}
CommentRemover::~CommentRemover() {}

void CommentRemover::Run() {
  std::cout<<"[CommentRemover][Run]"<<std::endl;
  std::vector<std::string> extension {"c", "h"};
  FileUtil::GetFilesByExtension(m_folder, m_files, extension);
  for (auto it=m_files.begin();it!=m_files.end();it++) {
    std::cout<<"[CommentRemover][Run][Debug]"<<*it<<std::endl;
    pugi::xml_document doc;
    SrcmlUtil::File2XML(*it, doc);
    pugi::xml_node root = doc.document_element();
    pugi::xpath_node_set comment_nodes = root.select_nodes("//comment");
    for (size_t i=0;i<comment_nodes.size();i++) {
      comment_nodes[i].parent().remove_child(comment_nodes[i].node());
    }
    // TODO Other more efficient method?
    std::string content = DomUtil::GetTextContent(root);
    std::ofstream of;
    of.open(*it);
    if (of.is_open()) {
      of<<content;
      of.close();
    }
  }
}
