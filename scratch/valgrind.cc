#include <pugixml.hpp>
#include <iostream>
/**
 * ./pugiparser out.xml <line-number>
 * return 0 if in out.xml, line-number is marked as InvalidRead or InvalidWrite
 * return 0 if in out.xml, line-number is marked as IR or IW, but also appear as first.
 */
int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "usage:" << "\n";
    std::cout <<"./valgrind-parser out.xml <line-number>"  << "\n";
    std::cout <<"return 0 is found."  << "\n";
  }
  pugi::xml_document doc;
  doc.load_file(argv[1]);
  for (auto error : doc.select_nodes("//error")) {
    pugi::xml_node error_node = error.node();
    std::string kind = error_node.child_value("kind");
    if (kind != "InvalidRead" && kind != "InvalidWrite") continue;
    for (pugi::xml_node frame : error_node.child("stack").children("frame")) {
      std::string filename = frame.child_value("file");
      std::string line = frame.child_value("line");
      if (line == argv[2]) return 0;
    }
  }
  return 1;
}

