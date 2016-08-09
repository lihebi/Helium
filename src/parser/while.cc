#include "ast_node.h"
#include "utils/log.h"

While::While(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- WHILE" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  XMLNode blk = while_get_block(xmlnode);
  // m_blk = new Block(blk_node, this, ast);
  for (XMLNode node : block_get_nodes(blk)) {
    ASTNode *n = ASTNodeFactory::CreateASTNode(node, this, ast);
    if (n) m_children.push_back(n);
  }
  m_cond = while_get_condition_expr(xmlnode);
  // m_children.push_back(m_blk);
}

void While::GetCode(std::set<ASTNode*> nodes,
                    std::string &ret, bool all) {
  // if (!nodes.empty() && nodes.count(this) == 0) return;
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += POIOutputCode();
    ret += "while (";
    ret += get_text(m_cond);
    ret += ")";
    ret += "{\n";
    for (ASTNode *child: m_children) {
      child->GetCode(nodes, ret, all);
    }
    ret += "}";
    ret += POIAfterCode();
  }
}

std::string While::GetLabel() {
  std::string ret;
  ret += "while (";
  ret += get_text(m_cond);
  ret += ")";
  return ret;
}

