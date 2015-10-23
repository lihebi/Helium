#include <pugixml.hpp>
#include <iostream>

int main() {
  std::cout << "start" << std::endl;
  pugi::xml_document doc;
  doc.load_file("srcml.xml");
  pugi::xml_node root = doc.document_element();
  pugi::xpath_node name = root.select_node("//*[@pos:line]");
  pugi::xml_node name_node = name.node();
  std::cout << name_node.attribute("pos:line") << std::endl;
  // pugi::xml_node node = root.select_node("//name").node();
  // std::cout << node.child_value() << std::endl;
}
