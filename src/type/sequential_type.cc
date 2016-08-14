#include "type.h"
#include "utils/log.h"
#include "type_helper.h"
#include "utils/utils.h"
#include "config/config.h"

/**
 * char aaa[5]
 * ArrayType(char, 5)
 */
ArrayType::ArrayType(std::string type_str, int num) {
  m_num = num;
  m_contained_type = TypeFactory::CreateType(type_str);
}

ArrayType::~ArrayType() {
  if (m_contained_type) {
    delete m_contained_type;
  }
}

std::string ArrayType::GetDeclCode(std::string var) {
  std::string ret;
  if (!m_contained_type) {
    helium_log_warning("ArrayType::GetDeclCode with no contained type");
    return "";
  }
  if (m_num == 0) {
    helium_log_warning("ArrayType::GetDeclCode array size is 0");
    return "";
  }
  ret += m_contained_type->GetDeclCode(var + "[" + std::to_string(m_num) + "]");
  return ret;
}

/**
 * If contained type is "char", use scanf("%s)
 * Otherwise, init the first index if available.
 */
std::string ArrayType::GetInputCode(std::string var) {
  std::string ret;
  if (!m_contained_type) {
    helium_log_warning("ArrayType::GetInputCode with no contained type");
    return "";
  }
  if (m_num == 0) {
    helium_log_warning("ArrayType::GetInputCode array size is 0");
    return "";
  }
  ret += m_contained_type->GetInputCode(var + "[0]");
  return ret;
}

std::string ArrayType::GetOutputCode(std::string var) {
  std::string ret;
  if (!m_contained_type) {
    helium_log_warning("ArrayType::GetOutputCode with no contained type");
    return "";
  }
  if (m_num == 0) {
    helium_log_warning("ArrayType::GetOutputCode array size is 0");
    return "";
  }
  ret += m_contained_type->GetOutputCode(var + "[0]");
  return ret;
}

InputSpec *ArrayType::GenerateInput() {
  InputSpec *spec = NULL;
  if (!m_contained_type) {
    helium_log_warning("ArrayType::GetOutputCode with no contained type");
    return NULL;
  }
  if (m_num == 0) {
    helium_log_warning("ArrayType::GetOutputCode array size is 0");
    return NULL;
  }
  spec = new ArrayInputSpec();
  InputSpec *tmp_spec = m_contained_type->GenerateInput();
  spec->Add(tmp_spec);
  return spec;
}


/********************************
 * PointerType
 *******************************/

PointerType::PointerType(std::string type_str) {
  m_contained_type = TypeFactory::CreateType(type_str);
}

PointerType::~PointerType() {
  if (m_contained_type) {
    delete m_contained_type;
  }
}

std::string PointerType::GetDeclCode(std::string var) {
  if (!m_contained_type) {
    helium_log_warning("PointerType::GetDeclCode with no contained type");
    return "";
  }
  return m_contained_type->GetDeclCode("*" + var);
}

std::string PointerType::GetInputCode(std::string var) {
  if (!m_contained_type) {
    helium_log_warning("PointerType::GetInputCode with no contained type");
    return "";
  }
  std::string ret;
  ret += get_scanf_code("%d", "&helium_size");
  ret += get_helium_size_branch(
                                // false branch, helium_size == 0
                                var + " = NULL;\n",
                                // get_malloc_code(var, m_raw_without_pointer, "1")
                                // GetValue() ?
                                get_malloc_code(var, m_contained_type->GetRaw(), "helium_size")
                                + m_contained_type->GetInputCode("*" + var)
                                );
  return ret;
}


std::string PointerType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// PointerType::GetOutputCode: " + var + "\n";
  ret += get_check_null(var,
                        get_null_output(var, true),
                        
                        get_null_output(var, false)
                        + m_contained_type->GetOutputCode("*" + var)
                        );
  return ret;
}

InputSpec *PointerType::GenerateInput() {
  if (!m_contained_type) {
    helium_log_warning("PointerType::GenerateInput with no contained type");
    return NULL;
  }
  InputSpec *spec = new PointerInputSpec();
  InputSpec *tmp_spec = m_contained_type->GenerateInput();
  spec->Add(tmp_spec);
  return spec;
}





/**
 * StrType
 */


StrType::StrType() : PointerType("char") {}

StrType::~StrType() {}

std::string StrType::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// StrType::GetDeclCode: " + var + "\n";
  ret += "char *" + var + ";\n";
  return ret;
}
std::string StrType::GetInputCode(std::string var) {
  std::string ret;
  ret += "// StrType::GetInputCode: " + var + "\n";
  ret += get_str_input_code(var);
  ret += get_addr_input(var);
  return ret;
}
std::string StrType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// StrType::GetOutputCode: " + var + "\n";
  ret += get_strlen_output(var);
  ret += get_addr_output(var);
  ret += get_check_null(var,
                        get_null_output(var, true),
                        get_null_output(var, false)
                        );
  return ret;
}
InputSpec *StrType::GenerateInput() {
  static int max_strlen = Config::Instance()->GetInt("max-strlen");
  // TODO at least a string? 0 here?
  int size = utils::rand_int(1, max_strlen);
  std::vector<std::string> list;
  list.push_back(std::to_string(size));
  std::string str = utils::rand_str(utils::rand_int(1, size));
  list.push_back(str);
  std::string spec = "{strlen: " + std::to_string(str.length())+ ", size: " + std::to_string(size) + "}";
  std::string joined = boost::algorithm::join(list, " ");
  std::string raw = joined;
  return new InputSpec(spec, raw);
}


/**
 * BufType
 */
BufType::BufType(int num) : ArrayType("char", num) {}

BufType::~BufType() {}

std::string BufType::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// BufType::GetDeclCode: " + var + ";\n";
  ret += "char " + var + "[" + std::to_string(m_num) + "];\n";
  return ret;
}
std::string BufType::GetInputCode(std::string var) {
  std::string ret;
  ret += "// BufType::GetInputCode: " + var + ";\n";
  ret += "// HELIUM_TODO\n";
  return ret;
}
std::string BufType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// BufType::GetOutputCode: " + var = "\n";
  ret += get_sizeof_output(var);
  ret += get_addr_output(var);
  return ret;
}
InputSpec *BufType::GenerateInput() {
  // TODO
  return NULL;
}




/**
 * Spec
 */



std::string ArrayInputSpec::GetSpec() {
  std::string ret;
  ret += "[";
  std::vector<std::string> list;
  for (InputSpec *spec : m_specs) {
    list.push_back(spec->GetSpec());
  }
  std::string joined = boost::algorithm::join(list, ", ");
  ret += joined;
  ret += "]";
  return ret;
}


std::string ArrayInputSpec::GetRaw() {
  std::string ret;
  std::vector<std::string> list;
  for (InputSpec *spec : m_specs) {
    list.push_back(spec->GetRaw());
  }
  std::string joined = boost::algorithm::join(list, " ");
  ret += joined;
  return ret;
}


std::string PointerInputSpec::GetSpec() {
  std::string ret;
  ret += "{HELIUM_POINTER: ";
  if (m_to) {
    ret += m_to->GetSpec();
  } else {
    ret += "NULL";
  }
  ret += "}";
  return ret;
}

