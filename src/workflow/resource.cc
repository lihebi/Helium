#include "resource.h"
#include "resolver/snippet_db.h"
#include "utils/log.h"
#include "config/options.h"
#include <iostream>

Resource* Resource::m_instance = 0;

XMLDoc *Resource::GetXMLDoc(std::string filename) {
  print_trace("Resource::GetXMLDoc");
  // file must exist
  XMLDoc *doc = XMLDocReader::CreateDocFromFile(filename);
  if (doc) {
    m_xmldocs[filename] = doc;
  }
  return doc;
}

AST* Resource::GetAST(std::string function) {
  print_trace("Resource::GetAST");
  if (m_asts.count(function) == 1) {
    return m_asts[function];
  }
  std::set<int> ids = SnippetDB::Instance()->LookUp(function, {SK_Function});
  if (ids.size() == 0) {
    helium_log("[WW] Resource::GetAST No snippet for function: " + function);
    return NULL;
  }
  if (ids.size() > 1) {
    helium_log("[WW] Resource::GetAST found more than one snippets for function: " + function);
  }
  int id = *ids.begin();
  XMLDoc *doc = XMLDocReader::Instance()->ReadSnippet(id);
  assert(doc);
  XMLNodeList funcs = find_nodes(doc->document_element(), NK_Function);
  if (funcs.size() == 0) {
    helium_log("[WW] Resource::GetAST the snippet code for function " + function + " does not contains a function");
    return NULL;
  }
  if (funcs.size() > 1) {
    helium_log("[WW] Resource::GetAST the snippet code for function " + function + " contains multiple functions");
  }
  XMLNode func = *funcs.begin();
  AST *ast = new AST(func);
  if (ast) {
    m_asts[function] = ast;
  }
  print_trace("Resource::GetAST end");
  return ast;
}

CFG* Resource::GetCFG(std::string function) {
  print_trace("Resource::GetCFG");
  if (m_cfgs.count(function) == 1) return m_cfgs[function];
  AST *ast = GetAST(function);
  CFG *cfg = CFGFactory::CreateCFG(ast);
  if (cfg) {
    m_cfgs[function] = cfg;
  }
  return cfg;
}

CFG* Resource::GetCFG(AST *ast) {
  if (!ast) return NULL;
  std::string function = ast->GetFunctionName();
  if (m_cfgs.count(function) == 1) return m_cfgs[function];
  CFG *cfg = CFGFactory::CreateCFG(ast);
  if (cfg) {
    m_cfgs[function] = cfg;
  }
  return cfg;
}
