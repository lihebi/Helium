#ifndef __VARIABLE_HPP__
#define __VARIABLE_HPP__

#include <pugixml/pugixml.hpp>
#include <vector>

#include "type/Type.hpp"

class Variable {
public:
  Variable();
  Variable(pugi::xml_node node);
  Variable(bool valid);
  ~Variable();
  operator bool() const {
    return m_valid;
  }
  const std::string& GetName() const {
    return m_name;
  }
  std::string GetInputCode() const;
  std::string GetOutputCode() const;
  void GetInputSpecification() const;
  void GetOutputSpecification() const;
  static void FromParamList(pugi::xml_node node, std::vector<Variable>& vv);
  static void FromForInit(pugi::xml_node node, std::vector<Variable>& vv);
private:
  pugi::xml_node m_node; // the node where the variable is declared
  std::shared_ptr<Type> m_type;
  std::string m_name;
  int dimension;
  bool m_valid;
};

#endif
