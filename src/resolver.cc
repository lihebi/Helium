#include "resolver.h"
#include <boost/regex.hpp>

static const std::set<std::string> c_common_keyword = {
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
  "sizeof", "return", "asm"
};

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
    if (c_common_keyword.find(tmp) == c_common_keyword.end()) {
      ss.insert(tmp);
    }
  }
  return ss;
}

bool
is_c_keyword(const std::string& s) {
  return c_common_keyword.find(s) != c_common_keyword.end();
}


SymbolTable::SymbolTable() {}
SymbolTable::~SymbolTable() {}
int SymbolTable::CurrentLevel() {
  return m_tables.size();
}
void SymbolTable::PushLevel() {
  m_tables.push_back(std::map<std::string, Variable>());
}
void SymbolTable::PopLevel() {
  m_tables.pop_back();
}
void SymbolTable::AddSymbol(Variable v) {
  m_tables.back()[v.Name()] = v;
}
void SymbolTable::AddSymbol(VariableList vars) {
  for (Variable v : vars.Variables()) {
    m_tables.back()[v.Name()] = v;
  }
}
Variable SymbolTable::LookUp(const std::string &name) {
  // FIXME back() will copy?
  // FIXME performance
  for (int i=m_tables.size()-1;i>=0;i--) {
    if (m_tables[i].find(name) != m_tables[i].end()) {
      return m_tables[i][name];
    }
  }
  return Variable();
}
