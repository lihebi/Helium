#include "type/SystemType.hpp"
#include <iostream>

SystemType::SystemType(
  const std::string& name, const struct struct_specifier& specifier
) : m_name(name), m_specifier(specifier) {
  std::cout << "[SystemType::SystemType] " << m_name << std::endl;
  if (m_specifier.is_struct)     m_type = "struct " + m_name;
  else if (m_specifier.is_enum)  m_type = "enum " + m_name;
  else if (m_specifier.is_union) m_type = "union " + m_name;
  else                           m_type = m_name;
}
SystemType::~SystemType() {
}

std::string
SystemType::GetInputCode(const std::string& var) const {
  std::string code;
  if (GetDimension()>0) {
    code = Type::GetArrayCode(m_type.c_str(), var, GetDimension());
  }
  if (GetPointerLevel() > 0) {
    code = Type::GetAllocateCode(m_type, var, GetPointerLevel());
  } else {
    code = m_type + " " + var + ";\n";
  }
  return code;
}

std::string
SystemType::GetOutputCode(const std::string& var) const {
  return "";
}
void SystemType::GetInputSpecification() {
}
void SystemType::GetOutputSpecification() {
}
