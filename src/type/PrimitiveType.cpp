#include "type/PrimitiveType.hpp"
#include "type/TypeFactory.hpp"
#include <iostream>
#include <cassert>

PrimitiveType::PrimitiveType(const struct type_specifier& specifier)
: m_specifier(specifier) {
  // m_name only as a printable name
  if (specifier.is_signed)   m_name += "signed ";
  if (specifier.is_unsigned) m_name += "unsigned ";
  if (specifier.is_short)    m_name += "short ";
  if (specifier.is_long)     m_name += "long ";
  if (specifier.is_int)      m_name += "int ";
  if (specifier.is_char)     m_name += "char ";
  if (specifier.is_float)    m_name += "float ";
  if (specifier.is_double)   m_name += "double ";
  if (specifier.is_bool)     m_name += "bool ";
  if (specifier.is_void)     m_name += "void ";
  // should at least have some specifier
  assert(!m_name.empty());
  m_name.pop_back();
}

PrimitiveType::~PrimitiveType() {}

static std::string
get_input(
  const std::string& type, const std::string& formatter,
  const std::string& var,
  int pointer_level, int dimension
) {
  std::string code;
  if (dimension>0) {
    return Type::GetArrayCode(type.c_str(), var, dimension);
  }
  std::string assign;
  if (pointer_level > 0) {
    code += Type::GetAllocateCode(type, var, pointer_level);
    assign = std::string(pointer_level-1, '*');
  } else {
    code += type + " " + var + ";\n";
    assign = "&";
  }
  code += "scanf(\"%" + formatter + "\", " + assign + var + ");\n";
  return code;
}

static std::string
get_char_input(const std::string& var, int pointer_level, int dimension) {
  std::string code;
  // TODO array of pointers?
  if (dimension > 0) {
    return Type::GetArrayCode("char", var, dimension);
  }
  // pointer or not
  std::string assign;
  if (pointer_level > 0) {
    code += Type::GetAllocateCode("char", var, pointer_level);
    assign = std::string(pointer_level-1, '*');
  } else {
    code += "char "+var+";\n";
    assign = "&";
  }
  code += "scanf(\"%c\", "+assign+var+");\n";
  return code;
}

std::string
get_void_input(const std::string& var, int pointer_level, int dimension) {
  if (pointer_level > 0) {
    return "void " + std::string(pointer_level, '*') + " " + var+" = NULL;\n";
  } else {
    std::cout << "[PrimitiveType::getVoidInputCode]"
    << "\033[31m" << "void should always be pointers" << "\033[0m" << std::endl;
    exit(1);
  }
}

std::string
PrimitiveType::GetInputCode(const std::string& var) const {
  if (m_specifier.is_char) return get_char_input(var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_float) return get_input("float", "f", var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_double) return get_input("double", "lf", var, GetPointerLevel(), GetDimension());
  // will not constrain bool here, but in input specification
  if (m_specifier.is_bool) return get_input("bool", "d", var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_void) return get_void_input(var, GetPointerLevel(), GetDimension());
  // rest is int
  return get_input("int", "d", var, GetPointerLevel(), GetDimension());
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
