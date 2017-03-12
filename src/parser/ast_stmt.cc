#include "helium/parser/ast_node.h"
#include "helium/parser/ast_common.h"
#include "helium/utils/log.h"
#include "helium/parser/ast_stmt.h"

Stmt::Stmt(XMLNode xmlnode, AST *ast, ASTNode *parent, ASTNode *prev)
  : ASTNode(xmlnode, ast, parent, prev) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- Stmt" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_prev = prev;
  m_ast = ast;

  
  CreateSymbolTable();
}

Stmt::~Stmt() {
  for (Decl *decl : m_decls) {
    delete decl;
  }
}

void Stmt::CreateSymbolTable() {
  // if (m_parent == NULL) {
  //   // this is root, create the default symbol table.
  //   m_sym_tbl = m_ast->CreateSymTbl(NULL);
  // } else {
  //   m_sym_tbl = m_parent->GetSymbolTable();
  // }
  // m_sym_tbl = m_ast->CreateSymTbl(NULL);
  /**
   * push the symbol table
   */
  if (xmlnode_to_kind(m_xmlnode) == NK_DeclStmt) {
    XMLNodeList decl_nodes = decl_stmt_get_decls(m_xmlnode);
    for (XMLNode decl_node : decl_nodes) {
      Decl *decl = DeclFactory::CreateDecl(decl_node);
      if (decl) {
        m_decls.push_back(decl);
        m_sym_tbl->AddSymbol(decl->GetName(), decl->GetType(), this);
      }
      // std::string declname = decl_get_name(decl);
      // std::string type = decl_get_type(decl);
      // m_sym_tbl->AddSymbol(declname, type, this);
    }
  }
}

void Stmt::GetCode(std::set<ASTNode*> nodes,
                   std::string &ret, bool all) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    // FIXME not only this, but also other model such as while, branch, because they can be the POI too
    // current ones: stmt, for, while, if
    ret += POIOutputCode();
    // TODO NOW add this to all the free point, but for this benchmark, it's fine, the code is a expr_stmt
    ret += FreedListCode();
    ret += get_text(m_xmlnode);
    // utils::print(get_text(m_xmlnode), utils::CK_Red);
    ret += "\n";
    ret += POIAfterCode();
  }
}

std::set<std::string> Stmt::GetIdToResolve() {
  std::string code;
  GetCode({}, code, true);
  std::set<std::string> ret = extract_id_to_resolve(code);
  return ret;
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
  return NULL;
}



/**
 * Function
 */


Function::Function(XMLNode xmlnode, AST *ast, ASTNode *parent, ASTNode *prev)
  : ASTNode(xmlnode, ast, parent, prev) {
  helium_print_trace("Function::Function");
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- Function" << "\n";
  #endif


  
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_prev = prev;
  m_ast = ast;
  // FIXME return static specifier
  m_ret_ty = function_get_return_type(xmlnode);
  m_name = xmlnode.child_value("name"); // function_get_name(xmlnode);


  CreateSymbolTable();
  
  // constructnig children
  XMLNode blk_n = xmlnode.child("block"); // function_get_block(xmlnode);
  // m_blk = new Block(blk_n, ast, this, prev);
  XMLNodeList nodes = block_get_nodes(blk_n);
  prev = NULL;
  for (XMLNode node : nodes) {
    ASTNode *anode = ASTNodeFactory::CreateASTNode(node, ast, this, prev);
    if (anode) {
      m_children.push_back(anode);
      prev = anode;
    }
  }
  // m_children.push_back(m_blk);
}

void Function::CreateSymbolTable() {
  XMLNodeList params = function_get_param_decls(m_xmlnode);
  for (XMLNode param : params) {
    Decl *decl = DeclFactory::CreateDecl(param);
    if (decl) {
      // needs to be delete'd
      m_params.push_back(decl);
    } else {
      helium_log_warning("Function::Function param is NULL");
    }
  }
  // if (m_parent == NULL) {
  //   // this is root, create the default symbol table.
  //   m_sym_tbl = m_ast->CreateSymTbl(NULL);
  // } else {
  //   m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymbolTable());
  // }
  /**
   * Push the symbol table
   */
  // std::cout << "function before adding: " << m_sym_tbl->ToStringLocal()  << "\n";
  for (Decl *decl : m_params) {
    m_sym_tbl->AddSymbol(decl->GetName(), decl->GetType(), this);
  }
  // std::cout << "function after adding: " << m_sym_tbl->ToStringLocal()  << "\n";
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
      // FIXME these parameter and the type's raw might not be the exact from the code
      // for example, the const will be removed
      // this will cause problem of inconsisitent function prototype
      // in the header file, the function is declared as is, with const
      // while in main.c the function parameter does not have const
      // The solution: TODO capture "const" in the type
      for (Decl *param : m_params) {
        ret += param->GetType()->GetRaw();
        ret += " ";
        ret += param->GetName();
        ret += ",";
      }
      ret.pop_back(); // remove last ","
    }
    ret += ")";
    ret += "{\n";
  }
  for (ASTNode *node : m_children) {
    node->GetCode(nodes, ret, all);
  }
  if (selected) {
    ret += "}";
  }
}

std::string Function::GetLabel() {
  // return m_name;
  std::string ret;
  ret += m_name + "(";
  if (!m_params.empty()) {
    for (Decl *param : m_params) {
      if (!param) {
        helium_log_warning("Function param is NULL");
        continue;
      }
      Type *type = param->GetType();
      if (!type) {
        helium_log_warning("Function param type is NULL");
        continue;
      }
      ret += type->GetRaw();
      ret += " ";
      ret += param->GetName();
      ret += ",";
    }
    ret.pop_back(); // remove last ","
  }
  ret += ")";
  return ret;
}

/**
 * Should not have the name of the function!
 */
std::set<std::string> Function::GetIdToResolve() {
  std::set<std::string> ret;
  std::set<std::string> tmp;
  tmp = extract_id_to_resolve(GetReturnType());
  ret.insert(tmp.begin(), tmp.end());
  for (Decl *param : m_params) {
    tmp = extract_id_to_resolve(param->GetType()->GetRaw());
    ret.insert(tmp.begin(), tmp.end());
  }
  return ret;
}

/**
 * Block
 */


Block::Block(XMLNode xmlnode, AST *ast, ASTNode *parent, ASTNode *prev)
  : ASTNode(xmlnode, ast, parent, prev) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- Block" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  m_prev = prev;

  // constructing children
  XMLNodeList nodes = block_get_nodes(xmlnode);
  for (XMLNode node : nodes) {
    ASTNode *anode = ASTNodeFactory::CreateASTNode(node, ast, this, prev);
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
  }
  for (ASTNode *n : m_children) {
    n->GetCode(nodes, ret, all);
  }
  if (selected) {
    ret += "}";
  }
}
