#ifndef __DOM_UTIL_HPP__
#define __DOM_UTIL_HPP__

#include <pugixml/pugixml.hpp>
#include <string>

class DomUtil {
public:
  static void GetFirstChildByTagName(std::string);
  static void GetFirstChildByTagNames(std::string);
  static void GetChildrenByTagName(std::string);

  static pugi::xml_node GetPreviousASTElement(pugi::xml_node node);
  static pugi::xml_node GetParentASTElement(pugi::xml_node node);
  static pugi::xml_node GetFunctionCall(pugi::xml_node node);
  static std::string GetTextContent(pugi::xml_node node);
};

#endif
