#ifndef __DOM_UTIL_HPP__
#define __DOM_UTIL_HPP__

#include <pugixml.hpp>
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

  // test if node is within <level> levels inside a <tagname>
  static bool InNode(pugi::xml_node node, std::string tagname, int level);
  // least upper bound of two nodes
  static pugi::xml_node lub(pugi::xml_node n1, pugi::xml_node n2);
};

#endif
