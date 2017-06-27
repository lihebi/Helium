#include "helium/parser/ast_cond.h"


/**
 * If
 */

If::If(XMLNode xmlnode, AST *ast, ASTNode *parent, ASTNode *prev)
  : ASTNode(xmlnode, ast, parent, prev) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- IF" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_prev = prev;
  m_ast = ast;
  m_cond = if_get_condition_expr(xmlnode);

  XMLNode then_node = if_get_then(xmlnode);
  XMLNode else_node = if_get_else(xmlnode);
  XMLNodeList nodes = if_get_elseifs(xmlnode);
  if (then_node) {
    m_then = new Then(then_node, ast, this, NULL);
    m_children.push_back(m_then);
  }
  for (XMLNode node : nodes) {
    ElseIf *anode = new ElseIf(node, ast, this, NULL);
    m_elseifs.push_back(anode);
  }
  m_children.insert(m_children.end(), m_elseifs.begin(), m_elseifs.end());
  if (else_node) {
    m_else = new Else(else_node, ast, this, NULL);
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
    ret += ") {\n";
  }
  
  assert(m_then); // then clause for a if must exist
  m_then->GetCode(nodes, ret, all);
  if (selected) {
    ret += "}\n";
  }
  
  for (ElseIf *ei : m_elseifs) {
    ei->GetCode(nodes, ret, all);
  }
  if (m_else) {
    m_else->GetCode(nodes, ret, all);
  }
  // FIXME the "after" point of a "branch POI" should be only surround condition?
  ret += POIAfterCode();

}

std::string If::GetLabel() {
  // with condition
  std::string ret;
  ret += "if (" + get_text(m_cond) + ")";
  return ret;
}


Then::Then(XMLNode xmlnode, AST *ast, ASTNode *parent, ASTNode *prev)
  : ASTNode(xmlnode, ast, parent, prev) {
 #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- THEN" << "\n";
  #endif
  m_parent = parent;
  m_prev = prev;
  m_xmlnode = xmlnode;
  m_ast = ast;

  XMLNode blk_node = then_get_block(xmlnode);
  for (XMLNode child : block_get_nodes(blk_node)) {
    ASTNode *node = ASTNodeFactory::CreateASTNode(child, m_ast, this, NULL);
    if (node) {
      m_children.push_back(node);
    }
  }
}


void Then::GetCode(std::set<ASTNode*> nodes,
                   std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "{\n";
    ret += "// then block\n";
  }
  for (ASTNode *n : m_children) {
    n->GetCode(nodes, ret, all);
  }
  if (selected) {
    ret += "}\n";
  }
}

Else::Else(XMLNode xmlnode, AST *ast, ASTNode *parent, ASTNode *prev)
  : ASTNode(xmlnode, ast, parent, prev) {
 #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- ELSE" << "\n";
  #endif
  m_parent = parent;
  m_prev = prev;
  m_xmlnode = xmlnode;
  m_ast = ast;

  XMLNode blk = else_get_block(xmlnode);
  for (XMLNode n : block_get_nodes(blk)) {
    ASTNode *node = ASTNodeFactory::CreateASTNode(n, m_ast, this, NULL);
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
  }
  for (ASTNode *n : m_children) {
    n->GetCode(nodes, ret, all);
  }
  if (selected) {
    ret += "}\n";
  }
}

ElseIf::ElseIf(XMLNode xmlnode, AST *ast, ASTNode *parent, ASTNode *prev)
  : ASTNode(xmlnode, ast, parent, prev) {
 #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- ELSEIF" << "\n";
  #endif
  m_parent = parent;
  m_prev = prev;
  m_xmlnode = xmlnode;
  m_ast = ast;
  m_cond = elseif_get_condition_expr(xmlnode);

  XMLNode blk = elseif_get_block(xmlnode);
  for (XMLNode n : block_get_nodes(blk)) {
    ASTNode *node = ASTNodeFactory::CreateASTNode(n, m_ast, this, NULL);
    if (node) {m_children.push_back(node);}
  }
}

void ElseIf::GetCode(std::set<ASTNode*> nodes,
                     std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "else if(" + get_text(m_cond) + ")";
    ret += "{\n";
  }
  for (ASTNode *n : m_children) {
    n->GetCode(nodes, ret, all);
  }
  if (selected) {
    ret += "}";
  }
}

std::string ElseIf::GetLabel() {
  std::string ret;
  ret += "elseif (" + get_text(m_cond) + ")";
  return ret;
}


/**
 * Switch
 */

/**
 * Test case

 switch(a) {
 case 1: s1; s2; s3; break;
 case 2:
 case 3: {s4; break;}
 default: break;
 }
*/

Switch::Switch(XMLNode xmlnode, AST *ast, ASTNode *parent, ASTNode *prev)
  : ASTNode(xmlnode, ast, parent, prev) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- SWITCH" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_prev = prev;
  m_ast = ast;

  // XMLNodeList blocks = switch_get_blocks(xmlnode);
  // for (XMLNode block_node : blocks) {
  //   Block* block = new Block(block_node, ast, this, prev);
  //   m_blks.push_back(block);
  // }
  // add cases
  XMLNodeList cases = switch_get_cases(xmlnode);
  for (XMLNode case_n : cases) {
    Case *c = new Case(case_n, ast, this, prev);
    m_cases.push_back(c);
  }
  m_cond = switch_get_condition_expr(xmlnode);
  m_children.insert(m_children.begin(), m_cases.begin(), m_cases.end());
  // add default
  XMLNode default_n = switch_get_default(xmlnode);
  if (default_n) {
    m_default = new Default(default_n, ast, this, prev);
    m_children.push_back(m_default);
  }
}

void Switch::GetCode(std::set<ASTNode*> nodes,
                     std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "switch (";
    ret += get_text(m_cond);
    ret += ")";
    ret += "{\n";
  }
  for (Case *c : m_cases) {
    c->GetCode(nodes, ret, all);
  }
  if (m_default) {
    m_default->GetCode(nodes, ret, all);
  }
  if (selected) {
    // add break at the end. It will not influence the result, but a label without statement is not syntax correct
    ret += "break;";
    ret += "}";
  }
}

std::string Switch::GetLabel() {
  return "switch (" + get_text(m_cond) + ")";
}

std::string Case::GetLabel() {
  return "case "+ get_text(m_cond);
}

Case::Case(XMLNode xmlnode, AST *ast, ASTNode *parent, ASTNode *prev)
  : ASTNode(xmlnode, ast, parent, prev) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- CASE" << "\n";
  #endif
  m_parent = parent;
  m_prev = prev;
  m_xmlnode = xmlnode;
  m_ast = ast;

  // from this case to next case or default
  XMLNode node = xmlnode.next_sibling();
  while (node && xmlnode_to_kind(node) != NK_Case && xmlnode_to_kind(node) != NK_Default) {
    ASTNode *n = ASTNodeFactory::CreateASTNode(node, ast, this, prev);
    if (n) m_children.push_back(n);
    node = node.next_sibling();
  }
  // get cond
  m_cond = case_get_condition_expr(xmlnode);
}

// void Case::CreateSymbolTable() {
//   // FIXME Should assert(m_parent!=NULL) ?
//   if (m_parent == NULL) {
//     // this is root, create the default symbol table.
//     m_sym_tbl = m_ast->CreateSymTbl(NULL);
//   } else {
//     // FIXME symbol table do not change
//     m_sym_tbl = m_parent->GetSymbolTable();
//   }
// }


void Case::GetCode(std::set<ASTNode*> nodes,
                   std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  // FIXME all cases should be selected in AST when doing completion
  // whenever there's any stmts selected in the switch
  if (selected) {
    ret += "case ";
    ret += get_text(m_cond);
    ret += ":\n";
  }
  for (ASTNode *child : m_children) {
    child->GetCode(nodes, ret, all);
  }
}

Default::Default(XMLNode xmlnode, AST *ast, ASTNode *parent, ASTNode *prev)
  : ASTNode(xmlnode, ast, parent, prev) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- DEFAULT" << "\n";
  #endif
  m_parent = parent;
  m_xmlnode = xmlnode;
  m_ast = ast;

  CreateSymbolTable();
  
  XMLNode node = xmlnode.next_sibling();
  while (node && xmlnode_to_kind(node) != NK_Case && xmlnode_to_kind(node) != NK_Default) {
    ASTNode *n = ASTNodeFactory::CreateASTNode(node, ast, this, prev);
    if (n) m_children.push_back(n);
    node = node.next_sibling();
  }
}
void Default::GetCode(std::set<ASTNode*> nodes, std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "default: ";
  }
  for (ASTNode *child : m_children) {
    child->GetCode(nodes, ret, all);
  }
}

