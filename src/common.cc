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

int global_seg_no = 0;
