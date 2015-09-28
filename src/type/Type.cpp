#include "type/Type.hpp"


std::string
Type::GetAllocateCode(const std::string& type_name, const std::string& var_name, int pointer_level) {
  std::string code;
  std::string var_tmp = var_name + "_tmp";
  code += type_name + "* " + var_tmp + " = (" + type_name + "*)malloc(sizeof(" + type_name + "));\n";
  code += type_name + std::string(pointer_level, '*')+ " " + var_name
  + " = " + std::string(pointer_level-1, '&') + var_tmp + ";\n";
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
