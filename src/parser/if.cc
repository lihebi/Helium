#include "ast_node.h"
#include "utils/log.h"

using namespace ast;
If::If(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- IF" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  m_cond = if_get_condition_expr(xmlnode);
  XMLNode then_node = if_get_then(xmlnode);
  XMLNode else_node = if_get_else(xmlnode);
  XMLNodeList nodes = if_get_elseifs(xmlnode);
  if (then_node) {
    m_then = new Then(then_node, this, ast);
    m_children.push_back(m_then);
  }
  for (XMLNode node : nodes) {
    ElseIf *anode = new ElseIf(node, this, ast);
    m_elseifs.push_back(anode);
  }
  m_children.insert(m_children.end(), m_elseifs.begin(), m_elseifs.end());
  if (else_node) {
    m_else = new Else(else_node, this, ast);
    m_children.push_back(m_else);
  }
}

void If::GetCode(std::set<ASTNode*> nodes,
                 std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += POIOutputCode();
    ret += "if (";
    ret += get_text(m_cond);
    ret += ")";
    assert(m_then); // then clause for a if must exist
    m_then->GetCode(nodes, ret, all);
    for (ElseIf *ei : m_elseifs) {
      ei->GetCode(nodes, ret, all);
    }
    if (m_else) {
      m_else->GetCode(nodes, ret, all);
    }
    // FIXME the "after" point of a "branch POI" should be only surround condition?
    ret += POIAfterCode();
  }
}

std::string If::GetLabel() {
  // with condition
  std::string ret;
  ret += "if (" + get_text(m_cond) + ")";
  return ret;
}


Then::Then(XMLNode xmlnode, ASTNode* parent, AST *ast) {
 #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- THEN" << "\n";
  #endif
  m_parent = parent;
  m_xmlnode = xmlnode;
  m_ast = ast;
  XMLNode blk_node = then_get_block(xmlnode);
  // m_blk = new Block(node, this, ast);
  // m_children.push_back(m_blk);
  for (XMLNode child : block_get_nodes(blk_node)) {
    ASTNode *node = ASTNodeFactory::CreateASTNode(child, this, m_ast);
    if (node) {
      m_children.push_back(node);
    }
  }
}


void Then::GetCode(std::set<ASTNode*> nodes,
                   std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  // For a then block, even if it is not selected, we need to have braces to make it compile
  ret += "{\n";
  if (selected) {
    for (ASTNode *n : m_children) {
      n->GetCode(nodes, ret, all);
    }
  }
  ret += "}\n";
}

Else::Else(XMLNode xmlnode, ASTNode* parent, AST *ast) {
 #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- ELSE" << "\n";
  #endif
   m_parent = parent;
  m_xmlnode = xmlnode;
  m_ast = ast;
  XMLNode blk = else_get_block(xmlnode);
  for (XMLNode n : block_get_nodes(blk)) {
    ASTNode *node = ASTNodeFactory::CreateASTNode(n, this, m_ast);
    if (node) {m_children.push_back(node);}
  }
}
void Else::GetCode(std::set<ASTNode*> nodes,
                   std::string &ret, bool all) {
  // if (!nodes.empty() && nodes.count(this) == 0) return;
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "else";
    ret += "{\n";
    for (ASTNode *n : m_children) {
      n->GetCode(nodes, ret, all);
    }
    ret += "}";
  }
}

ElseIf::ElseIf(XMLNode xmlnode, ASTNode* parent, AST *ast) {
 #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- ELSEIF" << "\n";
  #endif
   m_parent = parent;
  m_xmlnode = xmlnode;
  m_ast = ast;
  m_cond = elseif_get_condition_expr(xmlnode);
  // m_blk = new Block(node, this, ast);
  // m_children.push_back(m_blk);
  XMLNode blk = elseif_get_block(xmlnode);
  for (XMLNode n : block_get_nodes(blk)) {
    ASTNode *node = ASTNodeFactory::CreateASTNode(n, this, m_ast);
    if (node) {m_children.push_back(node);}
  }
}

void ElseIf::GetCode(std::set<ASTNode*> nodes,
                     std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "else if(" + get_text(m_cond) + ")";
    // put these {} inside (selected) because,
    // in the case of if() {} else if() {} else {}
    // the result would be
    // if () {} {} else {}, which is not syntax correct
    ret += "{\n";
    // m_blk->GetCode(nodes, ret, all, selected);
    for (ASTNode *n : m_children) {
      n->GetCode(nodes, ret, all);
    }
    if (selected) {
      ret += "}";
    }
  }
}

std::string ElseIf::GetLabel() {
  std::string ret;
  ret += "elseif (" + get_text(m_cond) + ")";
  return ret;
}


// ASTNode* If::LookUpDefinition(std::string id) {
//   // this is just condition, it is not a def node
//   std::string content = get_text(m_cond);
//   CheckDefKind k = check_def(content, id);
//   switch (k) {
//   case CDK_NULL: return NULL;
//   case CDK_This: return this;
//   case CDK_Continue: {
//     if (this->PreviousSibling())
//       return this->PreviousSibling()->LookUpDefinition(id);
//     else if (this->GetParent())
//       return this->GetParent()->LookUpDefinition(id);
//     else
//       return NULL;
//   }
//   default: assert(false);
//   }
// }
// ASTNode* ElseIf::LookUpDefinition(std::string id) {
//   std::string content = get_text(m_cond);
//   CheckDefKind k = check_def(content, id);
//   switch (k) {
//   case CDK_NULL: return NULL;
//   case CDK_This: return this;
//   case CDK_Continue: {
//     if (this->PreviousSibling())
//       return this->PreviousSibling()->LookUpDefinition(id);
//     else if (this->GetParent())
//       return this->GetParent()->LookUpDefinition(id);
//     else
//       return NULL;
//   }
//   }
//   return NULL;
// }
