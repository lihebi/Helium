#include "seg.h"
using namespace ast;

Seg::Seg(ast::XMLNode xmlnode) {
  XMLNode function_node = get_function_node(xmlnode);
  assert(function_node);
  AST *ast = getAST(function_node);
  // POI
  ASTNode *astnode = ast->GetNodeByXMLNode(xmlnode);
  m_nodes.insert(astnode);
  // Initial context
  m_ctxs.push_back(new Ctx(this));
}

Seg::~Seg() {
  for (auto m : m_asts_m) {
    delete m.second;
  }
  for (Ctx *ctx : m_ctxs) {
    delete ctx;
  }
}

/**
 * Get the next context.
 1. get the newest context
 2. get the first ASTNode
 3. get previous leaf node
 4. if interprocedure, query call graph, create a new AST and move from there
 */
void Seg::NextContext() {
  Ctx *last = m_ctxs.back();
  Ctx *ctx = new Ctx(*last);
  ASTNode *first_node = ctx->GetFirstNode();
  ASTNode *leaf = first_node->GetAST()->GetPreviousLeafNode(first_node);
  /**
   * Note: the leaf should not be a declaration of a variable, which will be included if the variable is used.
   * It will not contribute to the property, thus no need to tes.
   */
  if (leaf) {
    // leaf is found
    ctx->SetFirstNode(leaf);
  } else {
    // reach the function def
    // query the callgraph, find call-sites, try one callsite.
    // from that, form the new context
  }
  ctx->Test();
}

AST* Seg::getAST(XMLNode function_node) {
  assert(function_node && kind(function_node) == NK_Function);
  if (m_asts_m.count(function_node) == 0) {
    m_asts_m[function_node] = new AST(function_node);
  }
  return m_asts_m[function_node];
}


/**
 * Context
 */

void Ctx::Test() {
  // TODO
}
