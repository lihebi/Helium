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

  if (dynamic_cast<CharType*>(m_contained_type)) {
    ret += get_scanf_code("%s", var);
  } else {
    for (int i=0;i<m_num;i++) {
      ret += m_contained_type->GetInputCode(var + "[" + std::to_string(i) + "]");
    }
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
  if (dynamic_cast<CharType*>(m_contained_type)) {
    ret += get_strlen_printf_code(var);
  } else {
    for (int i=0;i<m_num;i++) {
      ret += m_contained_type->GetOutputCode(var + "[" + std::to_string(i) + "]");
    }
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
  if (dynamic_cast<CharType*>(m_contained_type)) {
    int len = utils::rand_int(0, m_num);
    std::string str = utils::rand_str(len);
    spec = new InputSpec("", str);
  } else {
    spec = new ArrayInputSpec();
    for (int i=0;i<m_num;i++) {
      InputSpec *tmp_spec = m_contained_type->GenerateRandomInput();
      spec->Add(tmp_spec);
    }
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
  ret += get_scanf_code_raw("%d", "&helium_size");

  
  std::string inner;
  inner += get_malloc_code(var, m_contained_type->GetRaw(), "helium_size");

  // record the size malloc-ed
  inner += "printf(\"malloc size for addr: %p is %d\\n\", (void*)"
    + var + LoopHelper::Instance()->GetSuffix()
    + ", helium_size);\n";
  // also store in the generated program, for later output instrumentation purpose
  inner += "helium_heap_addr[helium_heap_top]="
    + var + LoopHelper::Instance()->GetSuffix() + ";\n";
  inner += "helium_heap_size[helium_heap_top]=helium_size;\n";
  inner += "helium_heap_top++;\n";
  
  if (dynamic_cast<CharType*>(m_contained_type)) {
    // string inptu code
    inner += get_scanf_code("%s", var);
  } else {
    LoopHelper::Instance()->IncLevel();
    // the increased level will figure out the correct variable name
    std::string innerinner = m_contained_type->GetInputCode(var);
    inner += LoopHelper::Instance()->GetHeliumSizeLoop(innerinner);
    LoopHelper::Instance()->DecLevel();
  }
  // also I want to record the address, and the helium_size value, so that I can know the size of the buffer
  // But how to generate input?
  // It's easy: I generate the size, than generate that many input

  ret += get_helium_size_branch(var + LoopHelper::Instance()->GetSuffix() + " = NULL;\n",
                                inner);
  return ret;
}


std::string PointerType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// PointerType::GetOutputCode: " + var
    + " contained type: " + m_contained_type->ToString()
    +  "level: " + std::to_string(LoopHelper::Instance()->GetLevel()) + "\n";
  
  std::string inner;
  
  inner += get_isnull_printf_code(var, false);
  
  if (dynamic_cast<CharType*>(m_contained_type)) {
    inner += get_strlen_printf_code(var);
    inner += get_addr_printf_code(var);
    // use a new index variable
    LoopHelper::Instance()->IncLevel();
    inner += LoopHelper::Instance()->GetHeliumHeapCode(var, "");
    LoopHelper::Instance()->DecLevel();
  } else {
    // but for other types, output every contained type
    LoopHelper::Instance()->IncLevel();
    // here only use var, the increased level will figure out the correct variable name
    std::string innerinner = m_contained_type->GetOutputCode(var);
    inner += LoopHelper::Instance()->GetHeliumHeapCode(var, innerinner);
    LoopHelper::Instance()->DecLevel();
  }
  ret += get_check_null(var,
                        get_isnull_printf_code(var, true),
                        inner);
  return ret;
}

InputSpec *PointerType::GenerateRandomInput() {
  // std::cout << ToString() << "\n";
  InputSpec *ret = NULL;
  helium_print_trace("PointerType::GenerateRandomInput");
  if (!m_contained_type) {
    helium_log_warning("PointerType::GenerateRandomInput with no contained type");
    std::cerr << "EE: PointerType::GenerateRandomInput with no contained type" << "\n";
    return NULL;
  }

  if (dynamic_cast<CharType*>(m_contained_type)) {
    int max_strlen = HeliumOptions::Instance()->GetInt("test-input-max-strlen");
    int helium_size = utils::rand_int(0, max_strlen+1);
    std::string str;
    if (helium_size == 0) {
      str="";
    } else {
      int len = utils::rand_int(0, helium_size);
      str = utils::rand_str(len);
    }
    std::string raw = std::to_string(helium_size) + " " + str;
    std::string spec = "{strlen: " + std::to_string(str.length()) + ", size: " + std::to_string(helium_size) + "}";
    ret = new InputSpec(spec, raw);
  } else {
    ret = new PointerInputSpec();
    int max_pointer_size = HeliumOptions::Instance()->GetInt("test-input-max-pointer-size");
    int helium_size = utils::rand_int(0, max_pointer_size+1);
    for (int i=0;i<helium_size;i++) {
      InputSpec *tmp_spec = m_contained_type->GenerateRandomInput();
      ret->Add(tmp_spec);
    }
  }
  helium_print_trace("PointerType::GenerateRandomInput END");
  return ret;
}

std::vector<InputSpec*> PointerType::GeneratePairInput() {
  std::vector<InputSpec*> ret;
  // TODO
  return ret;
}
