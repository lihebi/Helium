#include "type/StructureType.hpp"

StructureType::StructureType(const std::string& name) : m_name(name) {
}
StructureType::~StructureType() {
}

std::string
StructureType::GetInputCode(const std::string& var) const {
  return "";
}

std::string
StructureType::GetOutputCode(const std::string& var) const {
  return "";
}
void StructureType::GetInputSpecification() {
}
void StructureType::GetOutputSpecification() {
}
