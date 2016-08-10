#include "type.h"
#include "type_common.h"
#include "type_helper.h"

using namespace type;
using namespace utils;

LocalType::LocalType(std::string raw, std::vector<std::string> dims) : Type(raw, dims) {
  std::set<int> snippet_ids = SnippetDB::Instance()->LookUp(m_id, {SK_Enum, SK_Structure});
  assert(snippet_ids.size() > 0);
  assert(snippet_ids.size() == 1);
  m_snippet_id = *snippet_ids.begin();
}

std::string LocalType::GetDeclCode(std::string var) {
  std::string ret;
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

/**
 * I need a more powerful local type input generation.
 * The features needed:
 * - analyze the fields and init important ones
 * - might have a shape, e.g. node->next
 */

std::string LocalType::GetInputCode(std::string var) {
  std::string ret;
  if (m_pointer == 0) {
    ret += "\n// HELIUM_TODO LocalType::GetInputCode\n";
  } else if (m_pointer == 1) {
    assert(m_dimension == 0 && "do not support array of pointer for now.");
    ret += "\n// LocalType::GetInputCode\n";
    // ret += get_scanf_code("%d", "&helium_size");
    // ret += get_malloc_code(var, "char", "helium_size");
    // ret += get_str_input_code(var);
    // this is 0 or 1, whether NULL or not null
    // FIXME so the generation of input should conform to this
    // FIXME is it safe to use helium_size? Nested one might change the value!
    ret += get_scanf_code("%d", "&helium_size");
    ret += get_helium_size_branch(
                                  // false branch, helium_size == 0
                                  var + " = NULL;\n",
                                  // 1. don't know how to init
                                  // 2. may need the shape: a->next=b;
                                  get_malloc_code(var, m_raw_without_pointer, "1")
                                  );
  } else {
    ret += "\n// HELIUM_TODO LocalType::GetInputCode\n";
  }
  return ret;
}

std::string LocalType::GetOutputCode(std::string var) {
  std::string ret;
  if (m_pointer == 0) {
    ret += "\n// HELIUM_TODO LocalType::GetOutputCode\n";
  } else if (m_pointer == 1) {
    if (Config::Instance()->GetBool("instrument-null")) {
      ret += get_check_null(var,
                            get_null_output(var, true),
                            get_null_output(var, false)
                            );
    }
  } else {
    ret += "\n// HELIUM_TODO LocalType::GetOutputCode\n";
  }
  return ret;
}

TestInput* LocalType::GetTestInputSpec(std::string var) {
  print_trace("LocalType::GetTestInputSpec(std::string var)");
  LocalTestInput *ret = new LocalTestInput(this, var);
  std::string raw;
  NullKind null = NULL_NA;
  if (m_pointer == 0) {
  } else if (m_pointer == 1) {
    bool res = rand_bool();
    if (res == true) {
      raw += "1";
      null = NULL_False; // !NULL
    } else {
      raw += "0";
      null = NULL_True; // NULL
    }
  } else {
  }
  ret->SetRaw(raw);
  ret->SetNull(null);
  return ret;
}



std::string LocalTestInput::dump() {
  std::string ret;
  ret += "LocalTestInput: ";
  assert(m_type);
  ret += m_type->ToString() + " " + m_var + "\n";
  return ret;
}

std::string LocalTestInput::ToString() {
  std::string ret;
  ret += "In_" + m_var + " = ";
  switch (m_null) {
  case NULL_NA: return "";
  case NULL_True: ret += "NULL\n"; break;
  case NULL_False: ret += "!NULL\n"; break;
  default: assert(false);
  }
  return ret;
}



