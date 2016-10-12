#ifndef RESOURCE_H
#define RESOURCE_H

#include "common.h"
#include "parser/xml_doc_reader.h"
#include "parser/xmlnode.h"
#include "parser/ast.h"
#include "parser/cfg.h"

/**
 * This contains the resources.
 * - XMLDoc
 * - AST for all the functions.
 * - CFG for all the functions.
 */

class Resource {
public:
  static Resource *Instance() {
    if (!m_instance) {
      m_instance = new Resource();
    }
    return m_instance;
  }

  XMLDoc *GetXMLDoc(std::string filename);
  AST* GetAST(int snippet_id);
  CFG* GetCFG(int snippet_id);
  CFG* GetCFG(AST* ast);
  int GetASTID(AST *ast) {
    if (ast && m_ast_ids.count(ast) == 1) {
      return m_ast_ids[ast];
    }
    return -1;
  }
private:
  Resource() {}
  ~Resource() {}
  static Resource* m_instance;
  
  std::map<std::string, XMLDoc*> m_xmldocs; // from filename to XMLDoc
  std::map<int, AST*> m_asts;
  std::map<AST*, int> m_ast_ids;
  std::map<AST*, CFG*> m_cfgs;


};

class StructClass {
public:
  StructClass(std::string code);
  ~StructClass() {}
  std::vector<Decl*> Fields() {return m_fields;}
private:
  std::vector<Decl*> m_fields;
};

class StructResource {
public:
  static StructResource* Instance() {
    if (!m_instance) {
      m_instance = new StructResource();
    }
    return m_instance;
  }
  StructClass *GetStruct(std::string name);
  StructClass *GetStruct(int snippet_id);
private:
  static StructResource *m_instance;
  std::map<int, StructClass*> m_iddata;
  std::map<std::string, StructClass*> m_namedata;
};

#endif /* RESOURCE_H */
