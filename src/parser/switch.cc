#include "ast_node.h"
#include "utils/log.h"

using namespace ast;
/**
 * Test case

 switch(a) {
 case 1: s1; s2; s3; break;
 case 2:
 case 3: {s4; break;}
 default: break;
 }
*/

Switch::Switch(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- SWITCH" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  // XMLNodeList blocks = switch_get_blocks(xmlnode);
  // for (XMLNode block_node : blocks) {
  //   Block* block = new Block(block_node, this, ast);
  //   m_blks.push_back(block);
  // }
  // add cases
  XMLNodeList cases = switch_get_cases(xmlnode);
  for (XMLNode case_n : cases) {
    Case *c = new Case(case_n, this, ast);
    m_cases.push_back(c);
  }
  m_cond = switch_get_condition_expr(xmlnode);
  m_children.insert(m_children.begin(), m_cases.begin(), m_cases.end());
  // add default
  XMLNode default_n = switch_get_default(xmlnode);
  if (default_n) {
    m_default = new Default(default_n, this, ast);
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
    for (Case *c : m_cases) {
      c->GetCode(nodes, ret, all);
    }
    if (m_default) {
      m_default->GetCode(nodes, ret, all);
    }
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

Case::Case(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- CASE" << "\n";
  #endif
  m_parent = parent;
  m_xmlnode = xmlnode;
  m_ast = ast;
  // from this case to next case or default
  XMLNode node = xmlnode.next_sibling();
  while (node && kind(node) != NK_Case && kind(node) != NK_Default) {
    ASTNode *n = ASTNodeFactory::CreateASTNode(node, this, ast);
    if (n) m_children.push_back(n);
    node = node.next_sibling();
  }
  // get cond
  m_cond = case_get_condition_expr(xmlnode);
}

void Case::CreateSymbolTable() {
  // FIXME Should assert(m_parent!=NULL) ?
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    // FIXME symbol table do not change
    m_sym_tbl = m_parent->GetSymbolTable();
  }
}


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
    for (ASTNode *child : m_children) {
      child->GetCode(nodes, ret, all);
    }
  }
}

Default::Default(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- DEFAULT" << "\n";
  #endif
  m_parent = parent;
  m_xmlnode = xmlnode;
  m_ast = ast;
  XMLNode node = xmlnode.next_sibling();
  while (node && kind(node) != NK_Case && kind(node) != NK_Default) {
    ASTNode *n = ASTNodeFactory::CreateASTNode(node, this, ast);
    if (n) m_children.push_back(n);
    node = node.next_sibling();
  }
}
void Default::GetCode(std::set<ASTNode*> nodes, std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "default: ";
    for (ASTNode *child : m_children) {
      child->GetCode(nodes, ret, all);
    }
  }
}


void Default::CreateSymbolTable() {
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_parent->GetSymbolTable();
  }
}
