#ifndef __VARIABLE_FACTORY_HPP__
#define __VARIABLE_FACTORY_HPP__

#include "variable/Variable.hpp"

class VariableFactory {
public:
  static std::vector<std::shared_ptr<Variable> > FromParamList(pugi::xml_node node);
  static std::vector<std::shared_ptr<Variable> > FromForInit(pugi::xml_node node);
  static std::shared_ptr<Variable> FromDecl(pugi::xml_node node);
  static std::shared_ptr<Variable> FromDeclStmt(pugi::xml_node node);
private:
};

#endif
