#include "Variable.hpp"
#include "util/DomUtil.hpp"
#include "type/TypeFactory.hpp"

void Variable::FromParamList(pugi::xml_node node, std::vector<Variable>& vv) {
  if (node.type() == pugi::node_element && strcmp(node.name(), "parameter_list") == 0) {
    for (pugi::xml_node param_node : node.children("param")) {
      vv.push_back(Variable(param_node.child("decl")));
    }
  }
}
void
// Deprecated for now
Variable::FromForInit(pugi::xml_node node, std::vector<Variable>& vv) {
  if (node.type() == pugi::node_element && strcmp(node.name(), "init") == 0) {
    // TODO support multiple variable in for init
  }
}

Variable::Variable() : m_valid(false) {
}
Variable::Variable(bool valid) : m_valid(valid) {}
Variable::Variable(pugi::xml_node node) : m_valid(false) {
  if (node.type() == pugi::node_element) {
    if (strcmp(node.name(), "decl_stmt") == 0) {
      node = node.child("decl");
    }
    if (strcmp(node.name(), "decl") == 0) {
      std::string type = DomUtil::GetTextContent(node.child("type"));
      std::string name = DomUtil::GetTextContent(node.child("name"));
      m_name = name.substr(0, name.find('['));
      // m_type = std::make_shared<Type>(type);
      m_type = TypeFactory::Instance()->CreateType(type);
      // TODO pointer, reference, array
    }
  }
}
Variable::~Variable() {
  ;
}

std::string Variable::GetInputCode() const {
  return m_type->GetInputCode(m_name);
}
std::string Variable::GetOutputCode() const {
  return "";
}
void Variable::GetInputSpecification() const {
  ;
}
void Variable::GetOutputSpecification() const {
  ;
}
