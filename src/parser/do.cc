#include "ast_node.h"
#include "utils/log.h"

using namespace ast;
Do::Do(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- DO" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  m_cond = while_get_condition_expr(xmlnode);
  XMLNode blk_node = while_get_block(xmlnode);
  for (XMLNode node : block_get_nodes(blk_node)) {
    ASTNode *n  = ASTNodeFactory::CreateASTNode(node, this, ast);
    if (n) {m_children.push_back(n);}
  }
  // m_blk = new Block(blk_node, this, ast);
  // m_children.push_back(m_blk);
}

void Do::GetCode(std::set<ASTNode*> nodes,
                 std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "do {\n";
    // m_blk->GetCode(nodes, ret, all, selected);
    for (ASTNode *child : m_children) {
      child->GetCode(nodes, ret, all);
    }
    ret += "} ";
    ret += "while (";
    ret += get_text(m_cond);
    ret += ");";
  }
}

std::string Do::GetLabel() {
  std::string ret;
  ret += "do ";
  ret += "while (";
  ret += get_text(m_cond);
  ret += ");";
  return ret;
}

