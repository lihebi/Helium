#include "type.h"

std::string UnknownType::GetInputCode(std::string var) {
  std::string ret;
  ret += "// HELIUM_TODO Uknown Type Input Code\n";
  return ret;
}
std::string UnknownType::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// HELIUM_TODO Uknown Type Decl Code\n";
  // TOOD dims
  ret += m_raw + " " + var + "\n";
  return ret;
}
std::string UnknownType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// HELIUM_TODO Uknown Type Output Code\n";
  return ret;
}
