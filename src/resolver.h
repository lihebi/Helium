#ifndef __RESOLVER_H__
#define __RESOLVER_H__

#include <readtags.h>
#include "common.h"
#include "snippet.h"
#include "ast.h"
#include "type.h"

std::set<std::string>
extract_id_to_resolve(const std::string& code);
std::set<std::string>
extract_id_to_resolve(ast::NodeList nodes);

std::set<std::string>
get_to_resolve(
               ast::NodeList nodes,
               std::set<std::string> known_to_resolve,
               std::set<std::string> known_not_resolve
               );

std::set<std::string>
get_to_resolve(
               std::string code,
               std::set<std::string> known_to_resolve = std::set<std::string>(),
               std::set<std::string> known_not_resolve = std::set<std::string>()
               );




class HeaderSorter {
public:
  static HeaderSorter* Instance() {
    if (m_instance == 0) {
      m_instance = new HeaderSorter();
    }
    return m_instance;
  }
  // load all the header files inside the folder recursively,
  // scan the #inlcude "" statement, and get dependence relations between them
  void Load(const std::string& folder);
  // sort the headers by dependence
  std::vector<std::string> Sort(std::set<std::string> headers);
  void Dump();
private:
  bool sortOneRound(std::vector<std::string> &sorted);
  bool isDependOn(std::string lhs, std::string rhs);
  void addDependence(const std::string& lhs, const std::string& rhs);
  HeaderSorter() {}
  ~HeaderSorter() {}
  static HeaderSorter* m_instance;

  // std::vector<std::string> m_headers;
  std::map<std::string, std::set<std::string> > m_dependence_map;
};



/**
 * This symbol table is only written for variable, no function support.
So specifically lookup will return Variable.
I need to look into how symbol table is implemented in llvm to decide how to support a fully qualified symbol table.
 */
class SymbolTable {
public:
  SymbolTable();
  ~SymbolTable();
  int CurrentLevel();
  void PushLevel();
  void PopLevel();
  void AddSymbol(Variable v);
  void AddSymbol(VariableList vars);
  Variable LookUp(const std::string &name);
private:
  std::vector<std::map<std::string, Variable> > m_tables;
};



namespace resolver {
  /*******************************
   ** variable
   *******************************/
  
  void get_alive_vars(ast::Node node, ast::NodeList nodes, VariableList &result);
  Variable resolve_var(ast::Node node, const std::string& name);
  void get_undefined_vars(ast::NodeList nodes, VariableList &result);
  void get_undefined_vars(ast::Node node, VariableList &result);
  void get_undefined_vars(ast::NodeList nodes, SymbolTable &st, VariableList &result);
  void get_undefined_vars(ast::Node node, SymbolTable &st, VariableList &result);
}

#endif
