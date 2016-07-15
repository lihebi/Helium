#include "type_common.h"

#include "type_helper.h"

std::string Int::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// Int::GetDeclCode\n";
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

/**
 * Count the occurence of pattern in orig
 */
int count_substr(std::string orig, std::string pattern) {
  int ret = 0;
  size_t pos = 0;
  while (
         (pos = orig.find(pattern, pos))
         != std::string::npos) {
    pos++;
    ret++;
  }
  return ret;
}

TEST(TmpCase, count_substr) {
  EXPECT_EQ(count_substr("helloworld", "wor"), 1);
  EXPECT_EQ(count_substr("foofoo", "foo"), 2);
  EXPECT_EQ(count_substr("helloworld", "foo"), 0);
}

std::string Int::GetInputCode(std::string var) {
  std::string ret;
  ret += "// Int::GetInputCode\n";
  int long_ct = count_substr(m_raw, "long");
  std::string formatter;
  
  if (long_ct == 0) formatter = "%d";
  else if (long_ct == 1) formatter = "%ld";
  else if (long_ct == 2) formatter = "%lld";
  else assert(false);
  
  if (m_pointer == 0) {
    // ret += get_scanf_code("%d", "&"+var);
    ret += type::get_scanf_code(formatter, "&"+var);
  } else if (m_pointer == 1) {
    // FIXME this shold also consider long long
    ret += type::get_malloc_code(var, "int", "helium_size");
  } else {
    assert(false && "More than int**?");
  }
  return ret;
}

std::string Int::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// Int::GetOutputCode\n";
  int long_ct = count_substr(m_raw, "long");
  std::string formatter;
  
  if (long_ct == 0) formatter = "%d";
  else if (long_ct == 1) formatter = "%ld";
  else if (long_ct == 2) formatter = "%lld";
  else assert(false);
  
  // TODO extract this into a helper function
  ret += "printf(\"Od_" + var
    // + " = %d\\n\", "
    + " = " + formatter + "\\n\", "
    + var + ");\n"
    + type::flush_output;
  return ret;
}

TestInput* Int::GetTestInputSpec(std::string var) {
  static int int_min = Config::Instance()->GetInt("int-min");
  static int int_max = Config::Instance()->GetInt("int-max");
  static int max_array_size = Config::Instance()->GetInt("max-array-size");
  assert(max_array_size > 0);

  int value = 0;
  std::string raw;
  if (m_pointer == 0) {
    int a = utils::rand_int(int_min, int_max);
    value = a;
    raw += std::to_string(a);
  } else if (m_pointer == 1) {
    int size = utils::rand_int(0, max_array_size);
    raw += std::to_string(size);
    for (int i=0;i<size;i++) {
      int a = utils::rand_int(int_min, int_max);
      raw += " " + std::to_string(a);
    }
  } else {
    assert(false);
  }
  IntTestInput *ret = new IntTestInput(this, var);
  ret->SetRaw(raw);
  ret->SetPointer(m_pointer);
  ret->SetValue(value);
  // TODO Set the value of string
  // TODO the dump method of IntTestInput
  return ret;
}



// for human read
std::string IntTestInput::dump() {
  std::string ret;
  ret += m_type->ToString() + " " + m_var + "\n";
  return ret;
}

std::string IntTestInput::ToString() {
  std::string ret;
  if (m_pointer == 0) {
    ret += "Id_" + m_var + " = " + std::to_string(m_value) + "\n";
  }
  return ret;
}
