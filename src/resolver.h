#ifndef __RESOLVER_H__
#define __RESOLVER_H__

#include <readtags.h>
#include "common.h"
#include "snippet.h"
#include "ast.h"
#include "type.h"

std::set<std::string>
extract_id_to_resolve(std::string code);
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
  // void addDependence(const std::string& lhs, const std::string& rhs);
  void implicit(std::string folder);
  void completeDeps();
  HeaderSorter() {}
  ~HeaderSorter() {}
  static HeaderSorter* m_instance;

  // std::vector<std::string> m_headers;
  std::map<std::string, std::set<std::string> > m_hard_deps_map;
  std::map<std::string, std::set<std::string> > m_soft_deps_map;
};



/**
 * This symbol table is only written for variable, no function support.
So specifically lookup will return Variable.
I need to look into how symbol table is implemented in llvm to decide how to support a fully qualified symbol table.
 */
class SymbolOldTable {
public:
  SymbolOldTable();
  ~SymbolOldTable();
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
  void get_undefined_vars(ast::NodeList nodes, SymbolOldTable &st, VariableList &result);
  void get_undefined_vars(ast::Node node, SymbolOldTable &st, VariableList &result);
}


/*
 * Check if an identifier is a system function or type.
 */

class SystemResolver {
public:
  static SystemResolver* Instance() {
    if (m_instance == 0) {
      m_instance = new SystemResolver();
    }
    return m_instance;
  }
  // load the systype.tags file
  void Load(const std::string& filename);
  // resolve to primitive type
  std::string ResolveType(const std::string& type);
  std::vector<CtagsEntry> Parse(const std::string& name) ;
  std::vector<CtagsEntry> Parse(const std::string& name, const std::string& type);
  bool Has(const std::string& name);
  std::string GetHeaders() const;
  std::string GetLibs() const;
  static void check_headers();
private:
  void parseHeaderConf(std::string file);
  SystemResolver();
  ~SystemResolver() {}
  // std::vector<Header> m_headers; // header files used
  static SystemResolver* m_instance;
  tagFile *m_tagfile = NULL;
  tagEntry *m_entry = NULL;
  // headers that need to be included
  std::set<std::string> m_headers;
  std::set<std::string> m_libs; // library compilation flags
};



#endif
