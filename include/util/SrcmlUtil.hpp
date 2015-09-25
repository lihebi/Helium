#ifndef __SRCML_UTIL_HPP__
#define __SRCML_UTIL_HPP__

#include <pugixml/pugixml.hpp>
#include <string>

class SrcmlUtil {
public:
  // filename is C source file. doc will be loaded with the xml
  static void File2XML(const std::string& filename, pugi::xml_document& doc);
  // c code will be converted into xml, and loaded by doc
  static void String2XML(const std::string& code, pugi::xml_document& doc);
};

#endif
