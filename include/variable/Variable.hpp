#ifndef __VARIABLE_HPP__
#define __VARIABLE_HPP__

#include <pugixml/pugixml.hpp>
#include <vector>

#include "type/Type.hpp"

class Variable {
public:
  Variable(std::shared_ptr<Type> type, const std::string& name);
  ~Variable() {}
  const std::string& GetName() const {
    return m_name;
  }
  const std::shared_ptr<Type> GetType() const { return m_type;}
  std::string GetInputCode() const;
  std::string GetOutputCode() const;
  void GetInputSpecification() const;
  void GetOutputSpecification() const;

private:
  pugi::xml_node m_node; // the node where the variable is declared
  std::shared_ptr<Type> m_type;
  std::string m_name;
};

#endif
