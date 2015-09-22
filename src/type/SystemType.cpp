#include "type/SystemType.hpp"
#include <iostream>

SystemType::SystemType(const std::string& name) : m_name(name) {
  std::cout << "[SystemType::SystemType] " << m_name << std::endl;
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
