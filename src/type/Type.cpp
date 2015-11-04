#include "type/Type.hpp"
#include <iostream>
#include <algorithm>

/*
 * Get decl code for a pointer type
 */
std::string
Type::GetDeclCode(const std::string& type_name, const std::string& var_name, int pointer_level) {
  return type_name + std::string(pointer_level, '*')+ " " + var_name+";\n";
}

static std::string
qualify_var_name(const std::string& varname) {
  std::string tmp = varname;
  tmp.erase(std::remove(tmp.begin(), tmp.end(), '.'));
  tmp.erase(std::remove(tmp.begin(), tmp.end(), '>'));
  tmp.erase(std::remove(tmp.begin(), tmp.end(), '-'));
  return tmp;
}
/**
 * Only get the allocate code(malloc, assign), but no decl code.
 */
std::string
Type::GetAllocateCode(const std::string& type_name, const std::string& var_name, int pointer_level) {
  std::string code;
  std::string var_tmp = qualify_var_name(var_name) + "_tmp";
  code += type_name + "* " + var_tmp + " = (" + type_name + "*)malloc(sizeof(" + type_name + "));\n";
  code += var_name + " = " + std::string(pointer_level-1, '&') + var_tmp + ";\n";
  return code;
}

std::string
Type::GetArrayCode(const std::string& type_name, const std::string& var_name, int dimension) {
  // TODO only support 1 dimension!!!
  std::string code;
  std::string size_var = var_name + "_size";
  // std::string
  code += "int " + size_var + ";\n";
  code += "scanf(\"%d\", &"+size_var + ");\n";
  code += type_name + " " + var_name + "[" + size_var + "];\n";
  // TODO init code
  // code += "for (int i=0;i<" + size_var + ";i++) {\n";
  // code += "   scanf()"
  return code;
}

void
Type::SetDimension(int d) {
  m_dimension = d;
}
int
Type::GetDimension() const {
  return m_dimension;
}
void
Type::SetPointerLevel(int l) {
  m_pointer_level = l;
}
int
Type::GetPointerLevel() const {
  return m_pointer_level;
}
