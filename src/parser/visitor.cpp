#include "helium/parser/visitor.h"

#include "helium/parser/ast_v2.h"
#include "helium/parser/source_manager.h"
#include "helium/utils/string_utils.h"
#include <iostream>

using std::vector;
using std::string;
using std::map;
using std::set;

using namespace v2;

void PreOrderVisitor::visit(v2::TokenNode *token, void *data) {
  visit_impl(token);
}
void PreOrderVisitor::visit(v2::TranslationUnitDecl *unit, void *data) {
  visit_impl(unit);
  std::vector<ASTNodeBase*> nodes = unit->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) {node->accept(this);}}}
void PreOrderVisitor::visit(v2::FunctionDecl *function, void *data) {
  visit_impl(function);
  TokenNode *ReturnTypeNode = function->getReturnTypeNode();
  if (ReturnTypeNode) ReturnTypeNode->accept(this);
  TokenNode *NameNode = function->getNameNode();
  if (NameNode) NameNode->accept(this);
  TokenNode *ParamNode = function->getParamNode();
  if (ParamNode) ParamNode->accept(this);
  Stmt *body = function->getBody();
  if (body) body->accept(this);
}
void PreOrderVisitor::visit(v2::DeclStmt *decl_stmt, void *data) {
  visit_impl(decl_stmt);
}
void PreOrderVisitor::visit(v2::ExprStmt *expr_stmt, void *data) {
  visit_impl(expr_stmt);
}
void PreOrderVisitor::visit(v2::CompoundStmt *comp_stmt, void *data) {
  visit_impl(comp_stmt);
  std::vector<Stmt*> stmts = comp_stmt->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) stmt->accept(this);}}
void PreOrderVisitor::visit(v2::ForStmt *for_stmt, void *data) {
  visit_impl(for_stmt);
  TokenNode *token = for_stmt->getForNode();
  if (token) token->accept(this);
  Expr *init = for_stmt->getInit();
  if (init) init->accept(this);
  Expr *cond = for_stmt->getCond();
  if (cond) cond->accept(this);
  Expr *inc = for_stmt->getInc();
  if (inc) inc->accept(this);
  Stmt *body = for_stmt->getBody();
  if (body) body->accept(this);
}
void PreOrderVisitor::visit(v2::WhileStmt *while_stmt, void *data) {
  visit_impl(while_stmt);
  TokenNode *WhileNode = while_stmt->getWhileNode();
  if (WhileNode) WhileNode->accept(this);
  Expr *cond = while_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
}
void PreOrderVisitor::visit(v2::DoStmt *do_stmt, void *data) {
  visit_impl(do_stmt);
  TokenNode *DoNode = do_stmt->getDoNode();
  if (DoNode) DoNode->accept(this);
  Stmt *body = do_stmt->getBody();
  if (body) body->accept(this);
  TokenNode *WhileNode = do_stmt->getWhileNode();
  if (WhileNode) WhileNode->accept(this);
  Expr *cond = do_stmt->getCond();
  if (cond) cond->accept(this);
}
void PreOrderVisitor::visit(v2::BreakStmt *break_stmt, void *data) {
  visit_impl(break_stmt);
}
void PreOrderVisitor::visit(v2::ContinueStmt *cont_stmt, void *data) {
  visit_impl(cont_stmt);
}
void PreOrderVisitor::visit(v2::ReturnStmt *ret_stmt, void *data) {
  visit_impl(ret_stmt);
  TokenNode *ReturnNode = ret_stmt->getReturnNode();
  if (ReturnNode) ReturnNode->accept(this);
  Expr *value = ret_stmt->getValue();
  if (value) value->accept(this);
}
void PreOrderVisitor::visit(v2::IfStmt *if_stmt, void *data) {
  visit_impl(if_stmt);
  TokenNode *IfNode = if_stmt->getIfNode();
  if (IfNode) IfNode->accept(this);
  Expr *expr = if_stmt->getCond();
  if (expr) expr->accept(this);
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) then_stmt->accept(this);
  TokenNode *ElseNode = if_stmt->getElseNode();
  if (ElseNode) ElseNode->accept(this);
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) else_stmt->accept(this);
}
void PreOrderVisitor::visit(v2::SwitchStmt *switch_stmt, void *data) {
  visit_impl(switch_stmt);
  TokenNode *SwitchNode = switch_stmt->getSwitchNode();
  if (SwitchNode) SwitchNode->accept(this);
  Expr *cond = switch_stmt->getCond();
  if (cond) cond->accept(this);
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  for (SwitchCase *c : cases) {
    if (c) c->accept(this);
  }
}
void PreOrderVisitor::visit(v2::CaseStmt *case_stmt, void *data) {
  visit_impl(case_stmt);
  TokenNode *CaseNode = case_stmt->getCaseNode();
  if (CaseNode) CaseNode->accept(this);
  Expr *cond = case_stmt->getCond();
  if (cond) cond->accept(this);
  vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void PreOrderVisitor::visit(v2::DefaultStmt *def_stmt, void *data) {
  visit_impl(def_stmt);
  TokenNode *DefaultNode = def_stmt->getDefaultNode();
  if (DefaultNode) DefaultNode->accept(this);
  vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void PreOrderVisitor::visit(v2::Expr *expr, void *data) {
  visit_impl(expr);
}






void TokenDumper::visit_impl(v2::ASTNodeBase *node) {
  node->dump(os);
}










template <class T> void CRTPVisitor<T>::visit(v2::TokenNode *token, void *data) {
  static_cast<T*>(this)->visit_pre(token);
  static_cast<T*>(this)->visit_post(token);
}
template <class T> void CRTPVisitor<T>::visit(v2::TranslationUnitDecl *unit, void *data) {
  static_cast<T*>(this)->visit_pre(unit);
  std::vector<ASTNodeBase*> nodes = unit->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) {node->accept(this);}
  }
  static_cast<T*>(this)->visit_post(unit);
}
template <class T> void CRTPVisitor<T>::visit(v2::FunctionDecl *function, void *data) {
  static_cast<T*>(this)->visit_pre(function);
  TokenNode *ReturnTypeNode = function->getReturnTypeNode();
  if (ReturnTypeNode) ReturnTypeNode->accept(this);
  TokenNode *NameNode = function->getNameNode();
  if (NameNode) NameNode->accept(this);
  TokenNode *ParamNode = function->getParamNode();
  if (ParamNode) ParamNode->accept(this);
  Stmt *body = function->getBody();
  if (body) body->accept(this);
  static_cast<T*>(this)->visit_post(function);
}
template <class T> void CRTPVisitor<T>::visit(v2::DeclStmt *decl_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(decl_stmt);
  static_cast<T*>(this)->visit_post(decl_stmt);
}
template <class T> void CRTPVisitor<T>::visit(v2::ExprStmt *expr_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(expr_stmt);
  static_cast<T*>(this)->visit_post(expr_stmt);
}
template <class T> void CRTPVisitor<T>::visit(v2::CompoundStmt *comp_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(comp_stmt);
  std::vector<Stmt*> stmts = comp_stmt->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) stmt->accept(this);
  }
  static_cast<T*>(this)->visit_post(comp_stmt);
}
template <class T> void CRTPVisitor<T>::visit(v2::ForStmt *for_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(for_stmt);
  TokenNode *token = for_stmt->getForNode();
  if (token) token->accept(this);
  Expr *init = for_stmt->getInit();
  if (init) init->accept(this);
  Expr *cond = for_stmt->getCond();
  if (cond) cond->accept(this);
  Expr *inc = for_stmt->getInc();
  if (inc) inc->accept(this);
  Stmt *body = for_stmt->getBody();
  if (body) body->accept(this);
  static_cast<T*>(this)->visit_post(for_stmt);
}
template <class T> void CRTPVisitor<T>::visit(v2::WhileStmt *while_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(while_stmt);
  TokenNode *WhileNode = while_stmt->getWhileNode();
  if (WhileNode) WhileNode->accept(this);
  Expr *cond = while_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
  static_cast<T*>(this)->visit_post(while_stmt);
}
template <class T> void CRTPVisitor<T>::visit(v2::DoStmt *do_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(do_stmt);
  TokenNode *DoNode = do_stmt->getDoNode();
  if (DoNode) DoNode->accept(this);
  Stmt *body = do_stmt->getBody();
  if (body) body->accept(this);
  TokenNode *WhileNode = do_stmt->getWhileNode();
  if (WhileNode) WhileNode->accept(this);
  Expr *cond = do_stmt->getCond();
  if (cond) cond->accept(this);
  static_cast<T*>(this)->visit_post(do_stmt);
}
template <class T> void CRTPVisitor<T>::visit(v2::BreakStmt *break_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(break_stmt);
  static_cast<T*>(this)->visit_post(break_stmt);
}
template <class T> void CRTPVisitor<T>::visit(v2::ContinueStmt *cont_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(cont_stmt);
  static_cast<T*>(this)->visit_post(cont_stmt);
}
template <class T> void CRTPVisitor<T>::visit(v2::ReturnStmt *ret_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(ret_stmt);
  TokenNode *ReturnNode = ret_stmt->getReturnNode();
  if (ReturnNode) ReturnNode->accept(this);
  Expr *value = ret_stmt->getValue();
  if (value) value->accept(this);
  static_cast<T*>(this)->visit_post(ret_stmt);
}
template <class T> void CRTPVisitor<T>::visit(v2::IfStmt *if_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(if_stmt);
  TokenNode *IfNode = if_stmt->getIfNode();
  if (IfNode) IfNode->accept(this);
  Expr *expr = if_stmt->getCond();
  if (expr) expr->accept(this);
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) then_stmt->accept(this);
  TokenNode *ElseNode = if_stmt->getElseNode();
  if (ElseNode) ElseNode->accept(this);
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) else_stmt->accept(this);
  static_cast<T*>(this)->visit_post(if_stmt);
}
template <class T> void CRTPVisitor<T>::visit(v2::SwitchStmt *switch_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(switch_stmt);
  TokenNode *SwitchNode = switch_stmt->getSwitchNode();
  if (SwitchNode) SwitchNode->accept(this);
  Expr *cond = switch_stmt->getCond();
  if (cond) cond->accept(this);
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  for (SwitchCase *c : cases) {
    if (c) c->accept(this);
  }
  static_cast<T*>(this)->visit_post(switch_stmt);
}
template <class T> void CRTPVisitor<T>::visit(v2::CaseStmt *case_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(case_stmt);
  TokenNode *CaseNode = case_stmt->getCaseNode();
  if (CaseNode) CaseNode->accept(this);
  Expr *cond = case_stmt->getCond();
  if (cond) cond->accept(this);
  vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
  static_cast<T*>(this)->visit_post(case_stmt);
}
template <class T> void CRTPVisitor<T>::visit(v2::DefaultStmt *def_stmt, void *data) {
  static_cast<T*>(this)->visit_pre(def_stmt);
  TokenNode *DefaultNode = def_stmt->getDefaultNode();
  if (DefaultNode) DefaultNode->accept(this);
  vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
template <class T> void CRTPVisitor<T>::visit(v2::Expr *expr, void *data) {
  static_cast<T*>(this)->visit_pre(expr);
  static_cast<T*>(this)->visit_post(expr);
}



void ASTDumper::visit_pre_general(v2::ASTNodeBase* node) {
  os << "(" << node->getBeginLoc().getLine() << ":" << node->getBeginLoc().getColumn() << " "
     << node->getNodeName();
}
void ASTDumper::visit_post_general(v2::ASTNodeBase* node) {
  os << ")";
}

void ASTDumper::visit_pre(v2::TokenNode *token) {visit_pre_general(token);}
void ASTDumper::visit_pre(v2::TranslationUnitDecl *unit) {visit_pre_general(unit);}
void ASTDumper::visit_pre(v2::FunctionDecl *function) {os<<"\n";visit_pre_general(function);os<<"\n";}
void ASTDumper::visit_pre(v2::DeclStmt *decl_stmt) {visit_pre_general(decl_stmt);}
void ASTDumper::visit_pre(v2::ExprStmt *expr_stmt) {visit_pre_general(expr_stmt);}
void ASTDumper::visit_pre(v2::CompoundStmt *comp_stmt) {os<<"\n";visit_pre_general(comp_stmt);os<<"\n";}
void ASTDumper::visit_pre(v2::ForStmt *for_stmt) {os<<"\n";visit_pre_general(for_stmt);os<<"\n";}
void ASTDumper::visit_pre(v2::WhileStmt *while_stmt) {os<<"\n";visit_pre_general(while_stmt);os<<"\n";}
void ASTDumper::visit_pre(v2::DoStmt *do_stmt) {visit_pre_general(do_stmt);os<<"\n";}
void ASTDumper::visit_pre(v2::BreakStmt *break_stmt) {visit_pre_general(break_stmt);}
void ASTDumper::visit_pre(v2::ContinueStmt *cont_stmt) {visit_pre_general(cont_stmt);}
void ASTDumper::visit_pre(v2::ReturnStmt *ret_stmt) {visit_pre_general(ret_stmt);}
void ASTDumper::visit_pre(v2::IfStmt *if_stmt) {os<<"\n";visit_pre_general(if_stmt);os<<"\n";}
void ASTDumper::visit_pre(v2::SwitchStmt *switch_stmt) {os<<"\n";visit_pre_general(switch_stmt);os<<"\n";}
void ASTDumper::visit_pre(v2::CaseStmt *case_stmt) {visit_pre_general(case_stmt);}
void ASTDumper::visit_pre(v2::DefaultStmt *def_stmt) {visit_pre_general(def_stmt);}
void ASTDumper::visit_pre(v2::Expr *expr) {visit_pre_general(expr);}

void ASTDumper::visit_post(v2::TokenNode *node) {visit_post_general(node);}
void ASTDumper::visit_post(v2::TranslationUnitDecl *node) {visit_post_general(node);os<<"\n";}
void ASTDumper::visit_post(v2::FunctionDecl *node) {visit_post_general(node);os<<"\n";}
void ASTDumper::visit_post(v2::DeclStmt *node) {visit_post_general(node);}
void ASTDumper::visit_post(v2::ExprStmt *node) {visit_post_general(node);}
void ASTDumper::visit_post(v2::CompoundStmt *node) {visit_post_general(node);os<<"\n";}
void ASTDumper::visit_post(v2::ForStmt *node) {visit_post_general(node);os<<"\n";}
void ASTDumper::visit_post(v2::WhileStmt *node) {visit_post_general(node);os<<"\n";}
void ASTDumper::visit_post(v2::DoStmt *node) {visit_post_general(node);os<<"\n";}
void ASTDumper::visit_post(v2::BreakStmt *node) {visit_post_general(node);}
void ASTDumper::visit_post(v2::ContinueStmt *node) {visit_post_general(node);}
void ASTDumper::visit_post(v2::ReturnStmt *node) {visit_post_general(node);}
void ASTDumper::visit_post(v2::IfStmt *node) {visit_post_general(node);os<<"\n";}
void ASTDumper::visit_post(v2::SwitchStmt *node) {visit_post_general(node);os<<"\n";}
void ASTDumper::visit_post(v2::CaseStmt *node) {visit_post_general(node);os<<"\n";}
void ASTDumper::visit_post(v2::DefaultStmt *node) {visit_post_general(node);os<<"\n";}
void ASTDumper::visit_post(v2::Expr *node) {visit_post_general(node);}





void LevelVisitorV2::visit_pre(v2::TokenNode *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::TranslationUnitDecl *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::FunctionDecl *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::DeclStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::ExprStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::CompoundStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::ForStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::WhileStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::DoStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::BreakStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::ContinueStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::ReturnStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::IfStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::SwitchStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::CaseStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::DefaultStmt *node) {visit_pre_general(node);}
void LevelVisitorV2::visit_pre(v2::Expr *node) {visit_pre_general(node);}

void LevelVisitorV2::visit_post(v2::TokenNode *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::TranslationUnitDecl *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::FunctionDecl *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::DeclStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::ExprStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::CompoundStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::ForStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::WhileStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::DoStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::BreakStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::ContinueStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::ReturnStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::IfStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::SwitchStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::CaseStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::DefaultStmt *node) {visit_post_general(node);}
void LevelVisitorV2::visit_post(v2::Expr *node) {visit_post_general(node);}



void SymbolTableBuilder::insertDefUse(v2::ASTNodeBase *use) {
  // for the use of variables, getvarid, and query symbol table
  // get_var_ids() requires a XMLNode, not good at all
  std::set<std::string> used_vars = use->getUsedVars();
  for (std::string var : used_vars) {
    v2::ASTNodeBase *def = Table.get(var);
    if (def) {
      Use2DefMap[use].insert(def);
    }
  }
}

/**
 * Add symbols
 */
void SymbolTableBuilder::visit_pre(v2::FunctionDecl *node) {
  Table.pushScope();
  std::set<std::string> vars = node->getVars();
  // FIXME add to function node or param node?
  Table.add(vars, node);
  insertDefUse(node);
}
void SymbolTableBuilder::visit_pre(v2::DeclStmt *node) {
  // introduce symbols
  std::set<std::string> vars = node->getVars();
  Table.add(vars, node);
  insertDefUse(node);
}
void SymbolTableBuilder::visit_pre(v2::ForStmt *node) {
  Table.pushScope();
  std::set<std::string> vars = node->getVars();
  Table.add(vars, node);
  insertDefUse(node);
}
/**
 * Add scopes
 */
void SymbolTableBuilder::visit_pre(v2::IfStmt *node) {
  // this is special
  // it adds two scopes
  // how should i do that?
  // oh, wait
  // it does not create any scope
  // it is the then and else COMPOUND Statement that creates scope
  // Yeah! do nothing here
  insertDefUse(node);
}
void SymbolTableBuilder::visit_pre(v2::TranslationUnitDecl *node) {Table.pushScope();insertDefUse(node);}
void SymbolTableBuilder::visit_pre(v2::CompoundStmt *node) {Table.pushScope();insertDefUse(node);}
void SymbolTableBuilder::visit_pre(v2::WhileStmt *node) {Table.pushScope();insertDefUse(node);}
void SymbolTableBuilder::visit_pre(v2::DoStmt *node) {Table.pushScope();insertDefUse(node);}
void SymbolTableBuilder::visit_pre(v2::SwitchStmt *node) {Table.pushScope();insertDefUse(node);}

/**
 * Nothing
 */
void SymbolTableBuilder::visit_pre(v2::TokenNode *node) {insertDefUse(node);}
void SymbolTableBuilder::visit_pre(v2::ExprStmt *node) {insertDefUse(node);}
void SymbolTableBuilder::visit_pre(v2::BreakStmt *node) {insertDefUse(node);}
void SymbolTableBuilder::visit_pre(v2::ContinueStmt *node) {insertDefUse(node);}
void SymbolTableBuilder::visit_pre(v2::ReturnStmt *node) {insertDefUse(node);}
void SymbolTableBuilder::visit_pre(v2::CaseStmt *node) {insertDefUse(node);}
void SymbolTableBuilder::visit_pre(v2::DefaultStmt *node) {insertDefUse(node);}
void SymbolTableBuilder::visit_pre(v2::Expr *node) {insertDefUse(node);}

/**
 * Post, for all the pushed scope, pop it out
 * TODO What i want to do with the symbol table?
 * After the visit, the symbol table will contain nothing
 * I must maintain what I want in between
 */
void SymbolTableBuilder::visit_post(v2::TranslationUnitDecl *node) {Table.popScope();}
void SymbolTableBuilder::visit_post(v2::FunctionDecl *node) {Table.popScope();}
void SymbolTableBuilder::visit_post(v2::CompoundStmt *node) {Table.popScope();}
void SymbolTableBuilder::visit_post(v2::ForStmt *node) {Table.popScope();}
void SymbolTableBuilder::visit_post(v2::WhileStmt *node) {Table.popScope();}
void SymbolTableBuilder::visit_post(v2::DoStmt *node) {Table.popScope();}
void SymbolTableBuilder::visit_post(v2::SwitchStmt *node) {Table.popScope();}





void SymbolTableBuilder::visit_post(v2::TokenNode *node) {}
void SymbolTableBuilder::visit_post(v2::DeclStmt *node) {}
void SymbolTableBuilder::visit_post(v2::ExprStmt *node) {}
void SymbolTableBuilder::visit_post(v2::BreakStmt *node) {}
void SymbolTableBuilder::visit_post(v2::ContinueStmt *node) {}
void SymbolTableBuilder::visit_post(v2::ReturnStmt *node) {}
void SymbolTableBuilder::visit_post(v2::IfStmt *node) {}
void SymbolTableBuilder::visit_post(v2::CaseStmt *node) {}
void SymbolTableBuilder::visit_post(v2::DefaultStmt *node) {}
void SymbolTableBuilder::visit_post(v2::Expr *node) {}
