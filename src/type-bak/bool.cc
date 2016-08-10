#include "type.h"
#include "type_common.h"
#include "type_helper.h"
using namespace type;

std::string Bool::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// Bool::GetDeclCode\n";
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

std::string Bool::GetInputCode(std::string var) {
  assert(m_pointer == 0);
  std::string ret;
  ret += "// Bool::GetInputCode: " + var + "\n";
  ret += "scanf(\"%d\", &" + var + ");\n";
  get_helium_size_branch(var + " = false",
                         var + " = true"
                         );
  return ret;
}

TestInput *Bool::GetTestInputSpec(std::string var) {
  std::string raw;
  BoolTestInput *ret = new BoolTestInput(this, var);
  bool v = utils::rand_bool();
  if (v) {
    raw += "1";
    ret->SetValue(true);
  } else {
    raw += "0";
    ret->SetValue(false);
  }
  ret->SetRaw(raw);
  return ret;
}

std::string BoolTestInput::dump() {
  std::string ret;
  ret += "Bool Test Input: " + m_type->ToString() + " : " + m_var + "\n";
  return ret;
}

std::string BoolTestInput::ToString() {
  std::string ret;
  ret += "Ib_" + m_var + " = "
    + (m_v==true?"true":"false") + "\n";
  return ret;
}

std::string Bool::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// Bool::GetOutputCode\n";
  ret += "printf(\"%d\\n\", " + var + ");\n" + flush_output;
  ret += "if (" + var + ") {\n";
  ret += "printf(\"Ib_" + var + " = true\\n\");\n";
  ret += "} else {";
  ret += "printf(\"Ib_" + var + " = false\\n\");\n";
  return ret;
}
