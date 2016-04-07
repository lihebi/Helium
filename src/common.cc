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

/** Extract id which is not c keyword
 * @param code [in] input code
 * @return a set of IDs
 */
std::set<std::string>
extract_id_to_resolve(const std::string& code) {
  static boost::regex id_reg("\\b[_a-zA-Z][_a-zA-Z0-9]*\\b");
  boost::smatch match;
  boost::sregex_iterator begin(code.begin(), code.end(), id_reg);
  boost::sregex_iterator end = boost::sregex_iterator();
  std::set<std::string> ss;
  for (boost::sregex_iterator it=begin;it!=end;it++) {
    std::string tmp = (*it).str();
    if (c_common_keywords.find(tmp) == c_common_keywords.end()) {
      ss.insert(tmp);
    }
  }
  return ss;
}
