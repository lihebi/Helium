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


// Visitor

void Visitor::visit(v2::TokenNode *token) {}
void Visitor::visit(v2::TranslationUnitDecl *unit) {
  std::vector<ASTNodeBase*> nodes = unit->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) {node->accept(this);}
  }
}
void Visitor::visit(v2::FunctionDecl *function) {
  TokenNode *ReturnTypeNode = function->getReturnTypeNode();
  if (ReturnTypeNode) ReturnTypeNode->accept(this);
  TokenNode *NameNode = function->getNameNode();
  if (NameNode) NameNode->accept(this);
  TokenNode *ParamNode = function->getParamNode();
  if (ParamNode) ParamNode->accept(this);
  Stmt *body = function->getBody();
  if (body) body->accept(this);
}
void Visitor::visit(v2::DeclStmt *decl_stmt) {}
void Visitor::visit(v2::ExprStmt *expr_stmt) {}
void Visitor::visit(v2::CompoundStmt *comp_stmt) {
  std::vector<Stmt*> stmts = comp_stmt->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) stmt->accept(this);
  }
}
void Visitor::visit(v2::ForStmt *for_stmt) {
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
void Visitor::visit(v2::WhileStmt *while_stmt) {
  TokenNode *WhileNode = while_stmt->getWhileNode();
  if (WhileNode) WhileNode->accept(this);
  Expr *cond = while_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
}
void Visitor::visit(v2::DoStmt *do_stmt) {
  TokenNode *DoNode = do_stmt->getDoNode();
  if (DoNode) DoNode->accept(this);
  Stmt *body = do_stmt->getBody();
  if (body) body->accept(this);
  TokenNode *WhileNode = do_stmt->getWhileNode();
  if (WhileNode) WhileNode->accept(this);
  Expr *cond = do_stmt->getCond();
  if (cond) cond->accept(this);
}
void Visitor::visit(v2::BreakStmt *break_stmt) {}
void Visitor::visit(v2::ContinueStmt *cont_stmt) {}
void Visitor::visit(v2::ReturnStmt *ret_stmt) {
  TokenNode *ReturnNode = ret_stmt->getReturnNode();
  if (ReturnNode) ReturnNode->accept(this);
  Expr *value = ret_stmt->getValue();
  if (value) value->accept(this);
}
void Visitor::visit(v2::IfStmt *if_stmt) {
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
void Visitor::visit(v2::SwitchStmt *switch_stmt) {
  TokenNode *SwitchNode = switch_stmt->getSwitchNode();
  if (SwitchNode) SwitchNode->accept(this);
  Expr *cond = switch_stmt->getCond();
  if (cond) cond->accept(this);
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  for (SwitchCase *c : cases) {
    if (c) c->accept(this);
  }
}
void Visitor::visit(v2::CaseStmt *case_stmt) {
  TokenNode *CaseNode = case_stmt->getCaseNode();
  if (CaseNode) CaseNode->accept(this);
  Expr *cond = case_stmt->getCond();
  if (cond) cond->accept(this);
  vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void Visitor::visit(v2::DefaultStmt *def_stmt) {
  TokenNode *DefaultNode = def_stmt->getDefaultNode();
  if (DefaultNode) DefaultNode->accept(this);
  vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void Visitor::visit(v2::Expr *expr) {
}
