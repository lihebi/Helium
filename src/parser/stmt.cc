#include "ast_node.h"
#include "ast_common.h"
#include "utils/log.h"

Stmt::Stmt(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- Stmt" << "\n";
  #endif
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
}

Stmt::~Stmt() {
  for (Decl *decl : m_decls) {
    delete decl;
  }
}

void Stmt::CreateSymbolTable() {
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_parent->GetSymbolTable();
  }
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
}
