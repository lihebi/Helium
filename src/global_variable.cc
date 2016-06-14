#include "global_variable.h"
#include "options.h"
#include <iostream>

using namespace ast;

GlobalVariableRegistry* GlobalVariableRegistry::m_instance = NULL;

GlobalVariableRegistry::GlobalVariableRegistry() {
  print_trace("GlobalVariableRegistry::GlobalVariableRegistry()");
  // create registry
  // 1. look up the snippet db, get all variables
  // 2. get the code
  // 3. parse the code into XML doc
  // 4. then, create the NewType
  std::set<int> snippet_ids = SnippetDB::Instance()->LookUp(SK_Variable);
  for (int id : snippet_ids) {
    std::string code = SnippetDB::Instance()->GetCode(id);
    ast::XMLDoc *doc = XMLDocReader::CreateDocFromString(code);
    XMLNodeList nodes = find_nodes(doc->document_element(), NK_Decl);
    for (XMLNode node : nodes) {
      std::string name = decl_get_name(node);
      if (m_variable_m.count(name) == 0) {
        NewType *type = NewTypeFactory::CreateType(node);
        m_variable_m[name] = new GlobalVariable(type, name, id);
      }
    }
  }
  // special variables
  NewType *type = NewTypeFactory::CreateType("char*", {});
  // FIXME I add optarg here, but, there're some good and bad things:
  // good: the variable is like a globla variable, no need to declare in generated program
  // bad: the snippet id is set to -1, which might cause crash when look up in snippet db
  m_variable_m["optarg"] = new GlobalVariable(type, "optarg", -1);
}

NewType *GlobalVariableRegistry::LookUp(std::string var) {
  print_trace("GlobalVariableRegistry::LookUp(std::string var)");
  if (m_variable_m.count(var) == 1) {
    return m_variable_m[var]->GetType();
  }
  return NULL;
}