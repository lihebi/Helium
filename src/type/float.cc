#include "type.h"
#include "type_common.h"
#include "type_helper.h"
using namespace type;

std::string Float::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// Float::GetDeclCode\n";
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

std::string Float::GetInputCode(std::string var) {
  std::string ret;
  ret += "// Float::GetInputCode\n";
  // a double should have pointer level just 0?
  assert(m_pointer == 0);
  ret += "scanf(\"%lf\", " + var + ");\n}";
  return ret;
}

std::string Float::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// Float::GetOutputCode\n";
  ret += "printf(\"Of_" + var + " = %f\\n\", " + var + ");\n" + flush_output;
  return ret;
}


