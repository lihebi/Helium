#include "helium/type/variable.h"

std::string Variable::GetDeclCode() {
  if (!m_is_global && m_type) {
    return m_type->GetDeclCode(m_var);
  }
  return "";
}
std::string Variable::GetInputCode() {
  if (m_type) {
    return m_type->GetInputCode(m_var);
  }
  return "";
}
std::string Variable::GetOutputCode() {
  if (m_type) {
    return m_type->GetOutputCode(m_var);
  }
  return "";
}
