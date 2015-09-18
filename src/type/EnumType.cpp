#include "type/EnumType.hpp"

EnumType::EnumType(const std::string& name) : m_name(name) {
}
EnumType::~EnumType() {
}
std::string
EnumType::GetInputCode(const std::string& var) const {
  return "";
}
std::string EnumType::GetOutputCode(const std::string& var) const {
  return "";
}
void EnumType::GetInputSpecification() {
}
void EnumType::GetOutputSpecification() {
}
