#include "ast_node.h"
#include "utils/log.h"

Block::Block(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- Block" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  // constructing children
  XMLNodeList nodes = block_get_nodes(xmlnode);
  for (XMLNode node : nodes) {
    ASTNode *anode = ASTNodeFactory::CreateASTNode(node, this, ast);
    if (anode) {
      // anode->SetParent(this);
      m_children.push_back(anode);
    }
  }
}

void Block::GetCode(std::set<ASTNode*> nodes,
                    std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "{\n";
    for (ASTNode *n : m_children) {
      n->GetCode(nodes, ret, all);
    }
    ret += "}";
  }
}
