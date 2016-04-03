#include "resolver.h"
#include <boost/regex.hpp>
#include "utils.h"


std::set<std::string>
extract_id_to_resolve(ast::NodeList nodes) {
  std::set<std::string> result;
  for (ast::Node node : nodes) {
    std::set<std::string> tmp = extract_id_to_resolve(ast::get_text(node));
    result.insert(tmp.begin(), tmp.end());
  }
  return result;
}

std::set<std::string>
get_to_resolve(
               ast::NodeList nodes,
               std::set<std::string> known_to_resolve,
               std::set<std::string> known_not_resolve
               ) {
  std::set<std::string> result;
  std::set<std::string> var_ids = ast::get_var_ids(nodes);
  result.insert(var_ids.begin(), var_ids.end());
  // var_ids
  // general types in the nodes
  std::set<std::string> type_ids = ast::get_type_ids(nodes);
  result.insert(type_ids.begin(), type_ids.end());
  // call to functions
  std::set<std::string> call_ids = ast::get_call_ids(nodes);
  result.insert(call_ids.begin(), call_ids.end());
  // constructing
  result.insert(known_to_resolve.begin(), known_to_resolve.end());
  for (const std::string &s : known_not_resolve) {
    result.erase(s);
  }
  for (const std::string &s : c_common_keywords) {
    result.erase(s);
  }
  return result;
}

/**
 * Semantic get to resolve set.
 */
std::set<std::string>
get_to_resolve(
               std::string code,
               std::set<std::string> known_to_resolve,
               std::set<std::string> known_not_resolve
               ) {
  ast::Doc doc;
  utils::string2xml(code, doc);
  ast::NodeList nodes;
  nodes.push_back(doc.document_element());
  return get_to_resolve(nodes, known_to_resolve, known_not_resolve);
}

SymbolTable::SymbolTable() {
  // ensure there's at least one table. Calling back() on empty vector is undefined.
  PushLevel();
}
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
  for (Variable v : vars) {
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
