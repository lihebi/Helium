#include "type/PrimitiveType.hpp"
#include "type/TypeFactory.hpp"
#include <iostream>

// Deprecated
PrimitiveType::PrimitiveType(const std::string& name)
: m_name(name) {
  ;
}

PrimitiveType::PrimitiveType(uint8_t length, uint8_t type)
: m_length(length), m_type(type) {
  if (length & UNSIGNED_MASK) m_name += "unsigned ";
  if (length & SIGNED_MASK) m_name += "signed ";
  if (length & SHORT_MASK) m_name += "short ";
  if (length & LONG_MASK) m_name += "long ";
  if (type & INT_MASK) m_name += "int ";
  if (type & CHAR_MASK) m_name += "char ";
  if (type & FLOAT_MASK) m_name += "float ";
  if (type & DOUBLE_MASK) m_name += "double ";
  if (type & BOOL_MASK) m_name += "bool ";
  if (type & VOID_MASK) m_name += "void";
  m_name.pop_back();
  // std::cout << "[PrimitiveType::PrimitiveType] " << m_name << std::endl;
}

PrimitiveType::~PrimitiveType() {}

std::string
PrimitiveType::getIntInputCode(const std::string& var) const {
  if (m_length & UNSIGNED_MASK) {
    // unsigned int
  } else if (m_length & SHORT_MASK) {
    // short
  } else if (m_length & LONG_MASK) {
    // long
  }
  std::string s = "int "+var+";\n";
  s += "scanf(\"%d\", &"+var+");\n";
  return s;
}

std::string
get_allocate_code(const std::string& type_name, const std::string& var_name, int pointer_level) {
  std::string code;
  std::string var_tmp = var_name + "_tmp";
  code += type_name + "* " + var_tmp + " = (" + type_name + "*)malloc(sizeof(" + type_name + "));\n";
  code += type_name + std::string(pointer_level, '*')+ " " + var_name
  + " = " + std::string(pointer_level-1, '&') + var_tmp + ";\n";
  return code;
}

std::string
PrimitiveType::getCharInputCode(const std::string& var) const {
  std::string code;
  std::string assign;
  if (m_pointer_level > 0) {
    code += get_allocate_code("char", var, m_pointer_level);
    assign = std::string(m_pointer_level-1, '*');
  } else {
    code += "char "+var+";\n";
    assign = "&";
  }
  code += "scanf(\"%c\", "+assign+var+");\n";
  return code;
}

std::string
PrimitiveType::getVoidInputCode(const std::string& var) const {
  if (m_pointer_level > 0) {
    // allocate
    return "void " + std::string(m_pointer_level, '*') + " " + var+";\n";
  } else {
    // for void type, it is always ...
    std::cout << "[PrimitiveType::getVoidInputCode] Critical Error, void xxx; exiting .." << std::endl;
    exit(1);
  }
}

std::string
PrimitiveType::GetInputCode(const std::string& var) const {
  std::string s;
  if (m_type) {
    if (m_type & INT_MASK) {
      return getIntInputCode(var);
    } else if (m_type & CHAR_MASK) {
      return getCharInputCode(var);
    } else if (m_type & FLOAT_MASK) {
      s += "float " + var + ";\n";
      s += "scanf(\"%f\", &"+var+");\n";
    } else if (m_type & DOUBLE_MASK) {
      s += "double " + var + ";\n";
      s += "scanf(\"%lf\", &"+var+");\n";
    } else if (m_type & BOOL_MASK) {
      s += "bool " + var + ";\n";
      s += "scanf(\"%d\", &"+var+");\n";
    } else if (m_type & VOID_MASK) {
      return getVoidInputCode(var);
    }
  } else {
    // int
    return getIntInputCode(var);
  }
  return s;
}

std::string
PrimitiveType::GetOutputCode(const std::string& var) const {
  return "";
}

void
PrimitiveType::GetInputSpecification() {
}

void
PrimitiveType::GetOutputSpecification() {
}
