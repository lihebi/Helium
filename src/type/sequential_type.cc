#include "type.h"
#include "utils/log.h"
#include "type_helper.h"
#include "utils/utils.h"
#include "corner.h"
#include "helium_options.h"
#include <iostream>

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
  for (int i=0;i<m_num;i++) {
    ret += m_contained_type->GetInputCode(var + "[" + std::to_string(i) + "]");
  }
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
  ret += get_sizeof_printf_code(var);
  ret += get_addr_printf_code(var);
  for (int i=0;i<m_num;i++) {
    ret += m_contained_type->GetOutputCode(var + "[" + std::to_string(i) + "]");
  }
  return ret;
}

InputSpec *ArrayType::GenerateRandomInput() {
  helium_print_trace("ArrayType::GenerateRandomInput");
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
  for (int i=0;i<m_num;i++) {
    InputSpec *tmp_spec = m_contained_type->GenerateRandomInput();
    spec->Add(tmp_spec);
  }
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
  std::string ret;
  ret += "// PointerType::GetDeclCode: " + var + "\n";
  ret += m_contained_type->GetDeclCode("*" + var);
  return ret;
}


/**
 *

 scanf(helium_size);
 var = malloc(helium_size);
 for (int i=0;i<helium_size;i++) {
   input(var[i]);
 }
 
 */
std::string PointerType::GetInputCode(std::string var) {
  if (!m_contained_type) {
    helium_log_warning("PointerType::GetInputCode with no contained type");
    return "";
  }
  std::string ret;
  ret += "// PointerType::GetInputCode: " + var + "\n";
  ret += get_scanf_code("%d", "&helium_size");
  std::string inner;
  inner += get_malloc_code(var, m_contained_type->GetRaw(), "helium_size");
  if (dynamic_cast<CharType*>(m_contained_type)) {
    // string inptu code
    inner += get_scanf_code("%s", var);
  } else {
    inner += "for (int i=0;i<helium_size;i++) {\n";
    inner +=   m_contained_type->GetInputCode(var + "[i]");
    inner += "}\n";
  }
  // also I want to record the address, and the helium_size value, so that I can know the size of the buffer
  // But how to generate input?
  // It's easy: I generate the size, than generate that many input
  ret += get_helium_size_branch(var + " = NULL;\n", // false branch, helium_size == 0
                                inner);
  return ret;
}


std::string PointerType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// PointerType::GetOutputCode: " + var + "\n";
  // TODO FIXME the output should output all the contained type
  // but I need to know the size first
  // how to know that?
  // 1. compare the address with whatever recorded in the malloc
  // 2. if match, use that size
  // This means:
  // each time we malloc a input variable, we store a map:
  // {address: size}
  // then here, we query the map, and retrieve the size
  // output according to the size

  std::string inner;
  inner += get_isnull_printf_code(var, false);
  if (dynamic_cast<CharType*>(m_contained_type)) {
    ret += get_strlen_printf_code(var);
    ret += get_addr_printf_code(var);
  } else {
    // TODO FIXME
    // m_contained_type->GetOutputCode("*" + var);
  }
  ret += get_check_null(var, get_isnull_printf_code(var, true), inner);
  return ret;
}

InputSpec *PointerType::GenerateRandomInput() {
  helium_print_trace("PointerType::GenerateRandomInput");
  if (!m_contained_type) {
    helium_log_warning("PointerType::GenerateRandomInput with no contained type");
    return NULL;
  }

  if (dynamic_cast<CharType*>(m_contained_type)) {
    int max_strlen = HeliumOptions::Instance()->GetInt("test-input-max-strlen");
    int helium_size = utils::rand_int(0, max_strlen+1);
    std::string str;
    if (helium_size == 0) {
      str="";
    } else {
      str = utils::rand_str(helium_size);
    }
    std::string raw = std::to_string(helium_size) + " " + str;
    std::string spec = "{strlen: " + std::to_string(str.length()) + ", size: " + std::to_string(helium_size) + "}";
    return new InputSpec(spec, raw);
  } else {
    InputSpec *ret = new PointerInputSpec();
    int max_pointer_size = HeliumOptions::Instance()->GetInt("test-input-max-pointer-size");
    int helium_size = utils::rand_int(0, max_pointer_size+1);
    for (int i=0;i<helium_size;i++) {
      InputSpec *tmp_spec = m_contained_type->GenerateRandomInput();
      ret->Add(tmp_spec);
    }
    return ret;
  }
  return NULL;
}

std::vector<InputSpec*> PointerType::GeneratePairInput() {
  std::vector<InputSpec*> ret;
  // TODO
  return ret;
}
























// /**
//  * StrType
//  */


// StrType::StrType() : PointerType("char") {
//   std::vector<int> strlen_corners = Corner::Instance()->GetStrlenCorner();
//   for (int len : strlen_corners) {
//     std::string str = utils::rand_str(len);
//     m_corners.push_back(wrap(str));
//   }
// }

// StrType::~StrType() {}

// std::string StrType::GetDeclCode(std::string var) {
//   std::string ret;
//   ret += "// StrType::GetDeclCode: " + var + "\n";
//   ret += "char *" + var + ";\n";
//   return ret;
// }
// std::string StrType::GetInputCode(std::string var) {

//   std::string ret;
//   ret += "// StrType::GetInputCode: " + var + "\n";
//   ret += get_scanf_code("%d", "&helium_size");
//   ret += get_malloc_code(var, "char", "helium_size");
//   ret += get_helium_size_branch(var + " = NULL;\n",
//                                 get_scanf_code("%s", var)
//                                 );
//   return ret;
// }
// std::string StrType::GetOutputCode(std::string var) {
//   std::string ret;
//   ret += "// StrType::GetOutputCode: " + var + "\n";
//   ret += get_strlen_printf_code(var);
//   ret += get_addr_printf_code(var);
//   ret += get_check_null(var,
//                         get_isnull_printf_code(var, true),
//                         get_isnull_printf_code(var, false)
//                         );
//   return ret;
// }
// InputSpec *StrType::GenerateRandomInput() {
//   helium_print_trace("StrType::GenerateRandomInput");
//   static int max_strlen = HeliumOptions::Instance()->GetInt("test-input-max-strlen");
//   // TODO at least a string? 0 here?
//   int size = utils::rand_int(1, max_strlen);
//   std::vector<std::string> list;
//   list.push_back(std::to_string(size));
//   std::string str = utils::rand_str(utils::rand_int(0, size));
//   list.push_back(str);
//   std::string spec = "{strlen: " + std::to_string(str.length())+ ", size: " + std::to_string(size) + "}";
//   std::string joined = boost::algorithm::join(list, " ");
//   std::string raw = joined;
//   return new InputSpec(spec, raw);
// }

// InputSpec *StrType::wrap(std::string str) {
//   std::vector<std::string> list;
//   list.push_back(std::to_string(str.size()));
//   list.push_back(str);
//   std::string spec = "{strlen: " + std::to_string(str.length())+ ", size: " + std::to_string(str.size()) + "}";
//   std::string joined = boost::algorithm::join(list, " ");
//   std::string raw = joined;
//   return new InputSpec(spec, raw);
// }

// std::vector<InputSpec*> StrType::GeneratePairInput() {
//   std::vector<InputSpec*> ret;
//   std::vector<int> strlen_corners = Corner::Instance()->GetStrlenCorner();
//   for (int len : strlen_corners) {
//     std::string str = utils::rand_str(len);
//     ret.push_back(wrap(str));
//   }
//   for (int i=0;i<5;i++) {
//     ret.push_back(GenerateRandomInput());
//   }
//   return ret;
// }


/**
 * BufType
 */
// BufType::BufType(int num) : ArrayType("char", num) {}

// BufType::~BufType() {}

// std::string BufType::GetDeclCode(std::string var) {
//   std::string ret;
//   ret += "// BufType::GetDeclCode: " + var + ";\n";
//   ret += "char " + var + "[" + std::to_string(m_num) + "];\n";
//   return ret;
// }
// std::string BufType::GetInputCode(std::string var) {
//   std::string ret;
//   ret += "// BufType::GetInputCode: " + var + ";\n";
//   ret += "// HELIUM_TODO\n";

//   // FIXME
//   // ret += get_sizeof_input_output(var);
//   return ret;
// }
// std::string BufType::GetOutputCode(std::string var) {
//   std::string ret;
//   ret += "// BufType::GetOutputCode: " + var = "\n";
//   ret += get_sizeof_printf_code(var);
//   ret += get_addr_printf_code(var);
//   return ret;
// }
// InputSpec *BufType::GenerateRandomInput() {
//   helium_print_trace("BufType::GenerateRandomInput");
//   // TODO
//   return NULL;
// }
