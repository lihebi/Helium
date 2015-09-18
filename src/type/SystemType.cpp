#include "type/SystemType.hpp"

SystemType::SystemType(const std::string& name) : m_name(name) {
}
SystemType::~SystemType() {
}

std::string
SystemType::GetInputCode(const std::string& var) const {
  return "";
}

std::string
SystemType::GetOutputCode(const std::string& var) const {
  return "";
}
void SystemType::GetInputSpecification() {
}
void SystemType::GetOutputSpecification() {
}
