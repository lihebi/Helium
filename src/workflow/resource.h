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
  AST *GetAST(std::string function);
  CFG *GetCFG(std::string function);
  CFG *GetCFG(AST *ast);
private:
  Resource() {}
  ~Resource() {}
  static Resource* m_instance;
  
  std::map<std::string, XMLDoc*> m_xmldocs;
  std::map<std::string, AST*> m_asts;
  std::map<std::string, CFG*> m_cfgs;
};

#endif /* RESOURCE_H */
