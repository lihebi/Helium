#include "ast_node.h"
#define DEBUG_AST_NODE_TRACE
#undef DEBUG_AST_NODE_TRACE
#include <iostream>
#include "utils.h"
#include <regex>

using namespace ast;

/**
 * This is NOT a ASTNode! Just itself.
 */
Decl::Decl(XMLNode xmlnode) {
  m_xmlnode = xmlnode;
  // m_type
  std::string type = decl_get_type(xmlnode);
  std::vector<std::string> dims = decl_get_dimension(xmlnode);
  m_type = NewTypeFactory::CreateType(type, dims);
  m_name = decl_get_name(xmlnode);
}

Decl::~Decl() {
  if (m_type) {
    delete m_type;
  }
}

/*******************************
 ** AST Node 
 *******************************/


/*******************************
 ** General
 *******************************/

Stmt::Stmt(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- Stmt" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_parent->GetSymbolTable();
  }
  /**
   * push the symbol table
   */
  if (kind(xmlnode) == NK_DeclStmt) {
    XMLNodeList decl_nodes = ast::decl_stmt_get_decls(xmlnode);
    for (XMLNode decl_node : decl_nodes) {
      Decl *decl = new Decl(decl_node);
      m_decls.push_back(decl);
      m_sym_tbl->AddSymbol(decl->GetName(), decl->GetType(), this);
      // std::string declname = decl_get_name(decl);
      // std::string type = decl_get_type(decl);
      // m_sym_tbl->AddSymbol(declname, type, this);
    }
  }
}

Stmt::~Stmt() {
  for (Decl *decl : m_decls) {
    delete decl;
  }
}

void Stmt::GetCode(std::set<ASTNode*> nodes,
                   std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    /**
     * Currently only do outupt before a Stmt.
     */
    // std::set<std::string> outputs = m_ast->GetRequiredDecoOutput(this);
    // for (std::string var : outputs) {
    //   SymbolTableValue *val = m_sym_tbl->LookUp(var);
    //   if (val) {
    //     std::string code = val->GetType()->GetOutputCode(val->GetName());
    //     ret += code;
    //   }
    // }

    std::vector<NewVariable> vars = m_ast->GetRequiredOutputVariables(this);
    if (vars.size() > 0) {
      ret += "printf(\"HELIUM_POI = true\\n\");\n";
      ret += "fflush(stdout);\n";
      for (NewVariable var : vars) {
        ret += var.GetType()->GetOutputCode(var.GetName());
      }
    }
    ret += get_text(m_xmlnode);
    // utils::print(get_text(m_xmlnode), utils::CK_Red);
    ret += "\n";
  } else {
    std::set<std::string> decls = m_ast->GetRequiredDecl(this);
    std::set<std::string> inputed_decls = m_ast->GetRequiredDeclWithInput(this);
    // std::cout << "this is a stmt"  << "\n";
    // std::cout << inputed_decls.size()  << "\n";
    // m_sym_tbl->dump();
    for (std::string decl : decls) {
      SymbolTableValue *val = m_sym_tbl->LookUp(decl);
      if (val) {
        // ret += "/*HELIUM_DECL*/";
        // ret += val->GetType() + " " + val->GetName() + ";\n";
        val->GetType()->GetDeclCode(val->GetName());
      }
    }
    // FIXME All the models lack this!
    for (std::string decl : inputed_decls) {
      SymbolTableValue *val = m_sym_tbl->LookUp(decl);
      if (val) {
        // ret += "/*HELIUM_DECL_WITH_INPUT*/";
        // ret += val->GetType() + " " + val->GetName() + ";\n";
        ret += val->GetType()->GetDeclCode(val->GetName());
        ret += val->GetType()->GetInputCode(val->GetName());
      }
    }
  }
  // ret += "\n";
}



/******************************
 ** Function
 *******************************/

Function::Function(XMLNode xmlnode, ASTNode *parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- Function" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymbolTable());
  }
  // m_ret_ty = xmlnode.child("type").child_value("name"); // function_get_return_type(xmlnode);
  m_ret_ty = function_get_return_type(xmlnode);
  m_name = xmlnode.child_value("name"); // function_get_name(xmlnode);
  XMLNodeList params = function_get_param_decls(xmlnode);
  for (XMLNode param : params) {
    // needs to be delete'd
    m_params.push_back(new Decl(param));
  }
  // constructnig children
  XMLNode blk_n = xmlnode.child("block"); // function_get_block(xmlnode);
  // m_blk = new Block(blk_n, this, ast);
  XMLNodeList nodes = block_get_nodes(blk_n);
  for (XMLNode node : nodes) {
    ASTNode *anode = ASTNodeFactory::CreateASTNode(node, this, ast);
    if (anode) {
      m_children.push_back(anode);
    }
  }
  // m_children.push_back(m_blk);
  /**
   * Push the symbol table
   */
  for (Decl *decl : m_params) {
    m_sym_tbl->AddSymbol(decl->GetName(), decl->GetType(), this);
  }
}

Function::~Function() {
  for (Decl *decl : m_params) {
    delete decl;
  }
}

void Function::GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += m_ret_ty + " " + m_name + "(";
    if (!m_params.empty()) {
      for (Decl *param : m_params) {
        ret += param->GetType()->Raw();
        ret += " ";
        ret += param->GetName();
        ret += ",";
      }
      ret.pop_back(); // remove last ","
    }
    ret += ")";
    ret += "{\n";

    // FIXME for now, function will never be returned by the GetCode
    // This is because I only care about intraprocedure for now, and the function definition inside main is not valid.
    for (ASTNode *node : m_children) {
      node->GetCode(nodes, ret, all);
    }
    // m_blk->GetCode(nodes, ret, all);
    ret += "}";
  } else {
    // decl
    // THIS is how to use the decls
    // the decls mean the declaration of a variable needs to be added to this node
    std::set<std::string> decls = m_ast->GetRequiredDecl(this);
    std::set<std::string> inputed_decls = m_ast->GetRequiredDeclWithInput(this);
    for (Decl *param: m_params) {
      std::string name = param->GetName();
      if (inputed_decls.count(name) == 1) {
        // TODO NOW inputed_decl need input statements
        // ret += "/*HELIUM_DECL_WITH_INPUT*/";
        // ret += param->GetType() + " " + param->GetName() + ";\n";
        ret += param->GetType()->GetDeclCode(param->GetName());
        ret += param->GetType()->GetInputCode(param->GetName());
      } else if (decls.count(name) == 1) {
        // ret += "/*HELIUM_DECL*/";
        // ret += param->GetType() + " " + param->GetName() + ";\n";
        ret += param->GetType()->GetDeclCode(param->GetName());
      }
    }
  }
}

std::string Function::GetLabel() {
  // return m_name;
  std::string ret;
  ret += m_name + "(";
  if (!m_params.empty()) {
    for (Decl *param : m_params) {
      ret += param->GetType()->Raw();
      ret += " ";
      ret += param->GetName();
      ret += ",";
    }
    ret.pop_back(); // remove last ","
  }
  ret += ")";
  return ret;
}

std::set<std::string> Function::GetIdToResolve() {
  std::set<std::string> ret;
  std::set<std::string> tmp;
  tmp = extract_id_to_resolve(GetReturnType());
  ret.insert(tmp.begin(), tmp.end());
  for (Decl *param : m_params) {
    tmp = extract_id_to_resolve(param->GetType()->Raw());
    ret.insert(tmp.begin(), tmp.end());
  }
  return ret;
}



Block::Block(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- Block" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymbolTable());
  }
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

/*******************************
 ** Condition
 *******************************/

If::If(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- IF" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymbolTable());
  }
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
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymbolTable());
  }
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
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymbolTable());
  }
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
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymbolTable());
  }
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
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymbolTable());
  }
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
  // FIXME Should assert(m_parent!=NULL) ?
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    // FIXME symbol table do not change
    m_sym_tbl = m_parent->GetSymbolTable();
  }
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
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_parent->GetSymbolTable();
  }
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

/*******************************
 ** Loop
 *******************************/

While::While(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- WHILE" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymbolTable());
  }
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
    ret += "while (";
    ret += get_text(m_cond);
    ret += ")";
    ret += "{\n";
    for (ASTNode *child: m_children) {
      child->GetCode(nodes, ret, all);
    }
    ret += "}";
  }
}

std::string While::GetLabel() {
  std::string ret;
  ret += "while (";
  ret += get_text(m_cond);
  ret += ")";
  return ret;
}



Do::Do(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- DO" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymbolTable());
  }
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


For::For(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- FOR" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymbolTable());
  }
  m_cond = for_get_condition_expr(xmlnode);
  m_incr = for_get_incr_expr(xmlnode);
  m_inits = for_get_init_decls_or_exprs(xmlnode);
  XMLNode blk_n = for_get_block(xmlnode);
  for (XMLNode node : block_get_nodes(blk_n)) {
    ASTNode *n  = ASTNodeFactory::CreateASTNode(node, this, ast);
    if (n) {m_children.push_back(n);}
  }
  /**
   * Push the symbol table
   */
  for (XMLNode init : m_inits) {
    if (kind(init) == NK_Decl) {
      Decl *decl = new Decl(init);
      m_decls.push_back(decl);
      m_sym_tbl->AddSymbol(decl->GetName(), decl->GetType(), this);
      // std::string name = decl_get_name(init);
      // std::string type = decl_get_type(init);
      // m_sym_tbl->AddSymbol(name, type, this);
    }
  }
}

For::~For() {
  for (Decl *decl : m_decls) {
    delete decl;
  }
}

void For::GetCode(std::set<ASTNode*> nodes,
                  std::string &ret, bool all) {
  // if (!nodes.empty() && nodes.count(this) == 0) return;
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "for (";
    // init
    if (!m_inits.empty()) {
      for (XMLNode n : m_inits) {
        ret += get_text(n);
        ret += ",";
      }
      ret.pop_back();
    }
    ret += ";" + get_text(m_cond) + ";" + get_text(m_incr) + ")";
    // m_blk->GetCode(nodes, ret, all, selected);
    ret += "{\n";
    for (ASTNode *child : m_children) {
      child->GetCode(nodes, ret, all);
    }
    ret += "}";
  } else {
    std::set<std::string> decls = m_ast->GetRequiredDecl(this);
    std::set<std::string> inputed_decls = m_ast->GetRequiredDeclWithInput(this);
    for (std::string decl : decls) {
      SymbolTableValue *val = m_sym_tbl->LookUp(decl);
      if (val) {
        // ret += "/*HELIUM_DECL*/";
        // ret += val->GetType() + " " + val->GetName() + ";\n";
        ret += val->GetType()->GetDeclCode(val->GetName());
      }
    }
    for (std::string decl : inputed_decls) {
      SymbolTableValue *val = m_sym_tbl->LookUp(decl);
      if (val) {
        // ret += "/*HELIUM_DECL_WITH_INPUT*/";
        // ret += val->GetType() + " " + val->GetName() + ";\n";
        ret += val->GetType()->GetDeclCode(val->GetName());
        ret += val->GetType()->GetInputCode(val->GetName());
      }
    }
  }
}

std::string For::GetLabel() {
  std::string ret;
  ret += "for (";
  // init
  if (!m_inits.empty()) {
    for (XMLNode n : m_inits) {
      ret += get_text(n);
      ret += ",";
    }
    ret.pop_back();
  }
  ret += ";" + get_text(m_cond) + ";" + get_text(m_incr) + ")";
  return ret;
}

std::set<std::string> For::GetVarIds() {
  std::set<std::string> ret;
  std::set<std::string> ids;
  ids = get_var_ids(m_cond);
  ret.insert(ids.begin(), ids.end());
  for (XMLNode init : m_inits) {
    ids = get_var_ids(init);
    ret.insert(ids.begin(), ids.end());
  }
  ids = get_var_ids(m_incr);
  ret.insert(ids.begin(), ids.end());
  return ret;
}


std::set<std::string> For::GetIdToResolve() {
  std::set<std::string> ret;
  std::set<std::string> tmp;
  tmp = extract_id_to_resolve(get_text(m_cond));
  ret.insert(tmp.begin(), tmp.end());
  tmp = extract_id_to_resolve(get_text(m_incr));
  ret.insert(tmp.begin(), tmp.end());
  for (XMLNode n : m_inits) {
    tmp = extract_id_to_resolve(get_text(n));
    ret.insert(tmp.begin(), tmp.end());
  }
  return ret;
}

/*******************************
 * LookUpDefinition
 *******************************/

typedef enum {
  CDK_NULL, // do not continue, because this variable is defined on itself. e.g. a = foo(a)
  CDK_This, // this node does defined this variable: a = foo(b,c)
  CDK_Continue // nothing found related to this variable, please continue search previous sibling and parent
} CheckDefKind;
/**
 * This only check the first "="
 */
CheckDefKind check_def(std::string code, std::string id) {
  // remove double ==
  for (size_t pos=code.find("==");
       pos!=std::string::npos;
       pos=code.find("==")) {
    code.erase(pos, 2);
  }
  // remove !=
  for (size_t pos=code.find("!=");
       pos!=std::string::npos;
       pos=code.find("!=")) {
    code.erase(pos, 2);
  }
  // find =, and treat left and right
  if (code.find('=') != std::string::npos) {
    std::string left = code.substr(0, code.find_first_of('='));
    std::string right = code.substr(code.find_first_of('='));
    // FIXME but the left may be *var
    std::regex reg("\\b" + id + "\\b");
    utils::trim(left);
    if (regex_search(left, reg) && left[0] != '*') {
      if (regex_search(right, reg)) {
        return CDK_NULL;
      } else {
        return CDK_This;
      }
    }
  }
  return CDK_Continue;
}

ASTNode* Stmt::LookUpDefinition(std::string id) {
  std::string content = get_text(m_xmlnode);
  CheckDefKind k = check_def(content, id);
  switch (k) {
  case CDK_NULL: return NULL;
  case CDK_This: return this;
  case CDK_Continue: {
    if (this->PreviousSibling())
      return this->PreviousSibling()->LookUpDefinition(id);
    else if (this->GetParent())
      return this->GetParent()->LookUpDefinition(id);
    else
      return NULL;
  }
  default: assert(false);
  }
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
ASTNode* For::LookUpDefinition(std::string id) {
  std::string code;
  for (XMLNode init : m_inits) {
    code = get_text(init);
    CheckDefKind k = check_def(code, id);
    if (k == CDK_NULL) return NULL;
    else if (k==CDK_This) return this;
  }
  code = get_text(m_cond);
  CheckDefKind k = check_def(code, id);
  if (k == CDK_NULL) return NULL;
  else if (k==CDK_This) return this;
  // I don't need to consider incr?
  if (this->PreviousSibling())
    return this->PreviousSibling()->LookUpDefinition(id);
  else if (this->GetParent())
    return this->GetParent()->LookUpDefinition(id);
  else
    return NULL;
}
