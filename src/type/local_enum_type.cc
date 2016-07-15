#include "type.h"
#include "type_common.h"
#include "type_helper.h"

using namespace type;
using namespace utils;

/**
 * Local Enum Type
 */


LocalEnumType::LocalEnumType(int snippet_id, std::string raw, std::vector<std::string> dims)
  : Type(raw, dims), m_snippet_id(snippet_id) {
  // std::set<int> snippet_ids = SnippetDB::Instance()->LookUp(m_id, {SK_Enum, SK_Enum});
  // assert(snippet_ids.size() > 0);
  // assert(snippet_ids.size() == 1);
  // m_snippet_id = *snippet_ids.begin();
}

std::string LocalEnumType::GetDeclCode(std::string var) {
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

std::string LocalEnumType::GetInputCode(std::string var) {
  std::string ret;
  if (m_pointer == 0) {
    ret += "\n// HELIUM_TODO LocalEnumType::GetInputCode\n";
  } else if (m_pointer == 1) {
    assert(m_dimension == 0 && "do not support array of pointer for now.");
    ret += "\n// LocalEnumType::GetInputCode\n";
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
    ret += "\n// HELIUM_TODO LocalEnumType::GetInputCode\n";
  }
  return ret;
}

std::string LocalEnumType::GetOutputCode(std::string var) {
  std::string ret;
  if (m_pointer == 0) {
    ret += "\n// HELIUM_TODO LocalEnumType::GetOutputCode\n";
  } else if (m_pointer == 1) {
    if (Config::Instance()->GetBool("instrument-null")) {
      ret += get_check_null(var,
                            get_null_output(var, true),
                            get_null_output(var, false)
                            );
    }
  } else {
    ret += "\n// HELIUM_TODO LocalEnumType::GetOutputCode\n";
  }
  return ret;
}



TestInput* LocalEnumType::GetTestInputSpec(std::string var) {
  TestInput *ret = new TestInput(this, var);
  ret->SetRaw("");
  return ret;
}
