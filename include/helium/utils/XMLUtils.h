#ifndef XML_UTILS_H
#define XML_UTILS_H

#include <vector>
#include <string>
#include <pugixml.hpp>

namespace utils {

  std::vector<std::string> query_xml(const std::string& xml_file, const std::string& query);
  std::string query_xml_first(const std::string& xml_file, const std::string& query);


  /*******************************
   ** Srcml Utils
   *******************************/

  pugi::xml_document* file2xml(const std::string &filename);
  void file2xml(const std::string& filename, pugi::xml_document& doc);
  void string2xml(const std::string& code, pugi::xml_document& doc);

}

#endif /* XML_UTILS_H */
