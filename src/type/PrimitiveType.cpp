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
get_input_decl(
  const std::string& type, const std::string& formatter,
  const std::string& var,
  int pointer_level, int dimension
) {
  std::string code;
  if (dimension>0) {
    return Type::GetArrayCode(type.c_str(), var, dimension);
  }
  if (pointer_level > 0) {
    code += Type::GetDeclCode(type, var, pointer_level);
    code += Type::GetAllocateCode(type, var, pointer_level);
  } else {
    code += type + " " + var + ";\n";
  }
  return code;
}

static std::string
get_input_scanf(
  const std::string& type, const std::string& formatter,
  const std::string& var,
  int pointer_level, int dimension
) {
  std::string assign;
  if (pointer_level > 0) {
    assign = std::string(pointer_level-1, '*');
  } else {
    assign = "&";
  }
  return "scanf(\"%" + formatter + "\", " + assign + var + ");\n";
}

static std::string
get_input(
  const std::string& type, const std::string& formatter,
  const std::string& var,
  int pointer_level, int dimension
) {
  std::string code;
  code += get_input_decl(type, formatter, var, pointer_level, dimension);
  code += get_input_scanf(type, formatter, var, pointer_level, dimension);
  return code;
}

static std::string
get_output(
  const std::string& type, const std::string& formatter,
  const std::string& var,
  int pointer_level, int dimension
) {
  std::string code;
  if (dimension > 0) {
    // code += "for (int _i=0;_i<"+var+"_size;_i++) {\n"
    //         "\tprintf(\"%" + formatter + "\\n\", " + var + "[_i]);\n}\n";
    // return code;
    // if it is array, we don't output anything
    return "";
  }
  if (pointer_level >0) {
    // if it is a pointer, only print if it is null
    // TODO print the value
    // This will pose difficulty on output specification.
    // This will also pose difficulty on run rate, because may cause buffer overflow
    return "printf(\"%d\\n\", ("+var+"==NULL));\n";
  }
  // code += "printf(\"%" + formatter + "\\n\", " + std::string(pointer_level, '*') + var + ");\n";
  code += "printf(\"%" + formatter + "\\n\", " + var + ");\n";
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
    code += Type::GetDeclCode("char", var, pointer_level);
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
    return "";
  }
}

std::string
get_void_output(const std::string& var, int pointer_level, int dimension) {
  if (pointer_level>0) {
    return "printf(\"%d\\n\", ("+var+"==NULL));\n";
  } else {
    // this should never happen, because already exit in get_void_input
    return "";
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
PrimitiveType::GetInputCodeWithoutDecl(const std::string& var) const {
  if (m_specifier.is_char) return get_input_scanf("char", "c", var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_float) return get_input_scanf("float", "f", var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_double) return get_input_scanf("double", "lf", var, GetPointerLevel(), GetDimension());
  // will not constrain bool here, but in input specification
  if (m_specifier.is_bool) return get_input_scanf("bool", "d", var, GetPointerLevel(), GetDimension());
  // if (m_specifier.is_void) return get_void_input(var, GetPointerLevel(), GetDimension());
  // rest is int
  return get_input_scanf("int", "d", var, GetPointerLevel(), GetDimension());
}

std::string
PrimitiveType::GetOutputCode(const std::string& var) const {
  // TODO char output
  // if (m_specifier.is_char) return get_char_output(var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_float) return get_output("float", "f", var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_double) return get_output("double", "lf", var, GetPointerLevel(), GetDimension());
  // will not constrain bool here, but in output specification
  if (m_specifier.is_bool) return get_output("bool", "d", var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_void) return get_void_output(var, GetPointerLevel(), GetDimension());
  // rest is int
  return get_output("int", "d", var, GetPointerLevel(), GetDimension());
}

std::string
PrimitiveType::GetInputSpecification() {
  std::string spec;

  if (m_specifier.is_char) spec += "50_70,";
  else if (m_specifier.is_float) spec += "0_100,";
  else if (m_specifier.is_double) spec += "0_100,";
  else if (m_specifier.is_bool) spec += "0_1,";
  else if (m_specifier.is_void) ;
  else {
    spec += "0_255,";
  }
  if (GetDimension()>0) {
    // spec = "size," + spec + "endsize";
    // we only support one dimension, as well as no nested size
    spec = "size," + spec;
  }
  if (spec.back() == ',') spec.pop_back();
  return spec;
}

std::string
PrimitiveType::GetOutputSpecification() {
  if (GetDimension()>0) return "";
  else if (GetPointerLevel()>0) {return "NULL";}
  else {
    if (m_specifier.is_char) return "char";
    else if (m_specifier.is_float) return "float";
    else if (m_specifier.is_double) return "double";
    else if (m_specifier.is_bool) return "bool";
    else return "int";
  }
}
