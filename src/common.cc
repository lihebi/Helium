#include "common.h"

const std::set<std::string> c_common_keywords = {
  // def, head
  "define", "undef", "ifdef", "ifndef",
  "main",  "include",
  // control branch keyword
  "if", "else", "switch", "case", "default", "for", "do", "while", "break", "goto", "break", "continue",
  // type
  "bool", "true", "false"
  // storage class specifier
  "auto", "register", "static", "extern", "typedef",
  // type specifier
  "void", "char", "short", "int", "long", "float", "double", "signed", "unsigned",
  "struct", "enum",
  // type qualifier
  "const", "volatile",
  // undefined
  "sizeof", "return", "asm", "NULL"
};

const std::set<std::string> c_extend_keywords = {
  "stderr", "stdout", "fprintf"
};

int global_seg_no = 0;
int g_compile_success_no = 0;
int g_compile_error_no = 0;

std::vector<int> g_data1;
std::vector<int> g_data2;

bool
is_c_keyword(const std::string& s) {
  if (c_common_keywords.count(s) == 1) return true;
  if (c_extend_keywords.count(s) == 1) return true;
  return false;
}

const std::string flush_output = "fflush(stdout);\n";


void error(std::string err_msg) {
  std::cerr << err_msg << "\n";
  exit(1);
}
