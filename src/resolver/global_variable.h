#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H


#include "type/type.h"
#include "parser/ast_node.h"
#include "resolver/snippet_db.h"
#include "parser/xml_doc_reader.h"

class GlobalVariable {
public:
  GlobalVariable(Type *type, std::string var, int snippet_id)
    : m_type(type), m_var(var), m_snippet_id(snippet_id) {}
  ~GlobalVariable() {
    // this actually will never be called
    delete m_type;
  }
  Type* GetType() {return m_type;}
  std::string GetName() {return m_var;}
  int GetSnippetID() {return m_snippet_id;}
private:
  Type *m_type;
  std::string m_var;
  int m_snippet_id;
};


/**
 * There're two kinds of global variable
 * - External Global Variables
 * - Static Global Variables
 */
class GlobalVariableRegistry {
public:
  static GlobalVariableRegistry *Instance() {
    if (!m_instance) {
      m_instance = new GlobalVariableRegistry();
    }
    return m_instance;
  }
  Type *LookUp(std::string var);
  ~GlobalVariableRegistry() {
    for (auto m : m_variable_m) {
      delete m.second;
    }
  }
private:
  GlobalVariableRegistry();
  static GlobalVariableRegistry *m_instance;
  std::map<std::string, GlobalVariable*> m_variable_m;
};


#endif /* GLOBAL_VARIABLE_H */
