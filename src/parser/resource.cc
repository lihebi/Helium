#include "resource.h"
#include "resolver/snippet_db.h"
#include "utils/log.h"
#include "parser/xmlnode_helper.h"
#include "ast_node.h"
#include <iostream>

Resource* Resource::m_instance = 0;

XMLDoc *Resource::GetXMLDoc(std::string filename) {
  helium_print_trace("Resource::GetXMLDoc");
  // file must exist
  XMLDoc *doc = XMLDocReader::CreateDocFromFile(filename);
  if (doc) {
    m_xmldocs[filename] = doc;
  }
  return doc;
}


AST* Resource::GetAST(int id) {
  helium_print_trace("Resource::GetAST");
  if (m_asts.count(id) == 1) {
    return m_asts[id];
  }
  // std::set<int> ids = SnippetDB::Instance()->LookUp(function, {SK_Function});
  // if (ids.size() == 0) {
  //   helium_print_warning("[WW] Resource::GetAST No snippet for function: " + function);
  //   return NULL;
  // }
  // if (ids.size() > 1) {
  //   helium_print_warning("[WW] Resource::GetAST found more than one snippets for function: " + function);
  // }
  // int id = *ids.begin();
  XMLDoc *doc = XMLDocReader::Instance()->ReadSnippet(id);
  assert(doc);
  XMLNodeList funcs = find_nodes(doc->document_element(), NK_Function);
  if (funcs.size() == 0) {
    helium_print_warning("[WW] Resource::GetAST the snippet code for function of ID: " + std::to_string(id) + " does not contains a function");
    return NULL;
  }
  if (funcs.size() > 1) {
    helium_print_warning("[WW] Resource::GetAST the snippet code for function of ID: " + std::to_string(id) + " contains multiple functions");
  }
  XMLNode func = *funcs.begin();
  AST *ast = new AST(func);
  if (ast) {
    m_asts[id] = ast;
    m_ast_ids[ast] = id;
  }
  helium_print_trace("Resource::GetAST end");
  return ast;
}

CFG* Resource::GetCFG(int id) {
  return GetCFG(GetAST(id));
}

CFG* Resource::GetCFG(AST *ast) {
  if (!ast) return NULL;
  if (m_cfgs.count(ast) == 1) return m_cfgs[ast];
  CFG *cfg = CFGFactory::CreateCFG(ast);
  if (cfg) {
    m_cfgs[ast] = cfg;
  }
  return cfg;
}




StructClass::StructClass(std::string code) {
  // fill in this:
  XMLDoc *doc = XMLDocReader::CreateDocFromString(code);
  // get the fields
  XMLNode node = find_first_node_bfs(*doc, "struct");
  assert(node);
  for (XMLNode decl_stmt_node : node.child("block").children("decl_stmt")) {
    for (XMLNode decl_node : decl_stmt_node.children("decl")) {
      Decl *decl = DeclFactory::CreateDecl(decl_node);
      m_fields.push_back(decl);
    }
  }
}


StructResource* StructResource::m_instance = NULL;
StructClass* StructResource::GetStruct(std::string name) {
  // TODO FIXME typedef? duplicate? enumerator?
  if (m_namedata.count(name) == 1) {
    return m_namedata[name];
  } else {
    std::set<int> ids = SnippetDB::Instance()->LookUp(name, {SK_Structure});
    for (int id : ids) {
      SnippetMeta meta = SnippetDB::Instance()->GetMeta(id);
    }
    int id = *ids.begin();
    return GetStruct(id);
  }
}
StructClass* StructResource::GetStruct(int snippet_id) {
  if (m_iddata.count(snippet_id) == 1) {
    return m_iddata[snippet_id];
  } else {
    std::string code = SnippetDB::Instance()->GetCode(snippet_id);
    SnippetMeta meta = SnippetDB::Instance()->GetMeta(snippet_id);
    StructClass *sc = new StructClass(code);
    if (sc) {
      std::string name = meta.GetKey();
      m_namedata[name] = sc;
      m_iddata[snippet_id] = sc;
    }
    return sc;
  }
}
