#include "parser/ast_node.h"
#include "utils/log.h"
#include <iostream>

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

