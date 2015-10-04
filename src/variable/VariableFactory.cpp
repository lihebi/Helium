#include "variable/VariableFactory.hpp"
#include "type/TypeFactory.hpp"
#include "util/DomUtil.hpp"
#include <iostream>

std::vector<std::shared_ptr<Variable> >
VariableFactory::FromParamList(pugi::xml_node node) {
  std::vector<std::shared_ptr<Variable> > vvp;
  if (node.type() == pugi::node_element && strcmp(node.name(), "parameter_list") == 0) {
    for (pugi::xml_node param_node : node.children("param")) {
      std::shared_ptr<Variable> vp = FromDecl(param_node.child("decl"));
      if (vp) vvp.push_back(vp);
    }
  }
  return vvp;
}

std::vector<std::shared_ptr<Variable> >
VariableFactory::FromForInit(pugi::xml_node node) {
  std::vector<std::shared_ptr<Variable> > vvp;
  if (node.type() == pugi::node_element && strcmp(node.name(), "init") == 0) {
    // TODO support multiple variable in for init
    node = node.child("decl");
    std::shared_ptr<Variable> vp = FromDecl(node);
    if (vp) vvp.push_back(vp);
  }
  return vvp;
}

std::shared_ptr<Variable>
VariableFactory::FromDeclStmt(pugi::xml_node node) {
  if (node.type() == pugi::node_element && strcmp(node.name(), "decl_stmt") == 0) {
    node = node.child("decl");
    return FromDecl(node);
  }
  return NULL;
}

std::shared_ptr<Variable>
VariableFactory::FromDecl(pugi::xml_node node) {
  if (node.type() == pugi::node_element && strcmp(node.name(), "decl") == 0) {
    // std::cout << "<decl>" << std::endl;
    // std::cout << DomUtil::GetTextContent(node) << std::endl;
    // node.print(std::cout);
    std::string type_str = DomUtil::GetTextContent(node.child("type"));
    std::string name_str = DomUtil::GetTextContent(node.child("name"));
    if (name_str.find('[') != -1) {
      type_str += name_str.substr(name_str.find('['));
    }
    name_str = name_str.substr(0, name_str.find('['));
    std::shared_ptr<Type> type = TypeFactory(type_str).CreateType();
    if (type) {
      return std::make_shared<Variable>(type, name_str);
    }

  }
  return NULL;
}
