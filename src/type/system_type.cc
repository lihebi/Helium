#include "type.h"

std::string SystemType::GetDeclCode(std::string var) {
  std::string ret;
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

std::string SystemType::GetInputCode(std::string var) {
  std::string ret;
  ret += "\n// HELIUM_TODO SystemType::GetInputCode\n";
  return ret;
}

std::string SystemType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "\n// HELIUM_TODO SystemType::GetOutputCode\n";
  return ret;
}
