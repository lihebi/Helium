#include "query.h"
#include "parser/resource.h"
#include "utils/log.h"
#include <iostream>
#include "code_gen.h"

std::set<CFGNode*> Query::m_bad = {};

Query::Query(ASTNode *astnode) {
  assert(astnode);
  CFG *cfg = Resource::Instance()->GetCFG(astnode->GetAST());
  CFGNode *cfgnode = cfg->ASTNodeToCFGNode(astnode);
  m_nodes.insert(cfgnode);
  m_new = cfgnode;
}

/**
 * Visualize on CFG.
 * Only display the first CFG for now.
 */
void Query::Visualize(bool open) {
  ASTNode *astnode = m_new->GetASTNode();
  CFG *cfg = Resource::Instance()->GetCFG(astnode->GetAST());
  // these nodes may not belong to this cfg
  cfg->Visualize(m_nodes, {m_new}, open);
}


void Query::ResolveInput() {
  helium_print_trace("Query::ResolveInput");
  // for all the ASTs, resolve input
  // TODO should we supply input when necessary? Based on the output instrumentation during run time?
  m_inputs.clear();
  // CFG *cfg = m_new->GetCFG();


  AST *ast = m_new->GetASTNode()->GetAST();


  // DEBUG
  // ast->Visualize2();


  
  // ASTNode *astnode = m_new->GetASTNode();
  // AST *ast = astnode->GetAST();
  std::set<ASTNode*> first_astnodes;
  // std::cout << "cfgnode size: " << m_nodes.size()  << "\n";
  for (CFGNode *cfgnode : m_nodes) {
    /// FIXME cfg is not consistent!
    // if (cfg->Contains(cfgnode)) {
    //   first_astnodes.insert(cfgnode->GetASTNode());
    // }
    ASTNode *astnode = cfgnode->GetASTNode();
    if (astnode && ast->Contains(astnode)) {
      first_astnodes.insert(astnode);
    }
  }
  // std::cout << "astnode size: " << first_astnodes.size()  << "\n";
  // std::map<std::string, Type*> inputs;
  for (ASTNode *astnode : first_astnodes) {
    std::set<std::string> ids = astnode->GetVarIds();
    // astnode->GetAST()->Visualize2();
    for (std::string id : ids) {
      if (id.empty()) continue;
      if (is_c_keyword(id)) continue;
      // std::cout << "  " << id  << "\n";
      SymbolTable *tbl = astnode->GetSymbolTable();
      SymbolTableValue *st_value = tbl->LookUp(id);
      if (st_value) {
        if (first_astnodes.count(st_value->GetNode()) == 0) {
          // input
          m_inputs[st_value->GetName()] = st_value->GetType();
        }
      } else {
        // TODO global
        // Type *type = GlobalVariableRegistry::Instance()->LookUp(id);
      }
    }
  }
}


/**
 * Generate main, support, makefile, scripts
 */
void Query::GenCode() {
  CodeGen generator;
  // generator.SetFirstAST(m_new->GetASTNode()->GetAST());
  generator.SetFirstNode(m_new->GetASTNode());
  for (CFGNode *cfgnode : m_nodes) {
    generator.AddNode(cfgnode->GetASTNode());
  }
  generator.SetInput(m_inputs);
  generator.Compute();
  m_main = generator.GetMain();
  m_support = generator.GetSupport();
  m_makefile = generator.GetMakefile();
}

