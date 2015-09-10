#ifndef __VARIABLE_HPP__
#define __VARIABLE_HPP__

#include <pugixml/pugixml.hpp>

#include "type/Type.hpp"

class Variable {
public:
  Variable();
  ~Variable();
  void GetInputCode();
  void GetOutputCode();
  void GetInputSpecification();
  void GetOutputSpecification();
private:
  pugi::xml_node m_node; // the node where the variable is declared
  std::shared_ptr<Type> m_type;
  std::string m_name;
};

#endif
