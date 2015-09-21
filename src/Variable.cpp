#include "variable/Variable.hpp"
#include "util/DomUtil.hpp"
#include "type/TypeFactory.hpp"



Variable::Variable() {}
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
