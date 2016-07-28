#include "type.h"
#include "type_common.h"
#include "type_helper.h"
using namespace type;

std::string Void::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// Void::GetDeclCode\n";
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

std::string Void::GetInputCode(std::string var) {
  std::string ret;
  ret += "// Void::GetInputCode\n";
  var = "No use";
  assert(m_pointer > 0 && "A void should be a pointer");
  // what to be generate here?
  // assert(false && "I'm not clear whether to generate input for a void*");
  return ret;
}
std::string Void::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// Void::GetOutputCode\n";
  // TODO check if it is NULL
  ret += "printf(\"%d\\n\", " + var + ");\n" = flush_output;
  return ret;
}
