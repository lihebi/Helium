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



void TokenVisitor::visit(v2::TokenNode *token, void *data) {
  IdMap[token] = id;
  Tokens.push_back(token);
  id++;
}
void TokenVisitor::visit(v2::TranslationUnitDecl *unit, void *data) {
  std::vector<ASTNodeBase*> nodes = unit->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) node->accept(this);
  }
}
void TokenVisitor::visit(v2::FunctionDecl *function, void *data) {
  TokenNode *ReturnNode = function->getReturnTypeNode();
  if (ReturnNode) ReturnNode->accept(this);
  TokenNode *NameNode = function->getNameNode();
  if (NameNode) NameNode->accept(this);
  TokenNode *ParamNode = function->getParamNode();
  if (ParamNode) ParamNode->accept(this);
  Stmt *body = function->getBody();
  if (body) body->accept(this);
}
void TokenVisitor::visit(v2::DeclStmt *decl_stmt, void *data) {
  IdMap[decl_stmt] = id;
  Tokens.push_back(decl_stmt);
  id++;
}
void TokenVisitor::visit(v2::ExprStmt *expr_stmt, void *data) {
  IdMap[expr_stmt] = id;
  Tokens.push_back(expr_stmt);
  id++;
}
void TokenVisitor::visit(v2::CompoundStmt *comp_stmt, void *data) {
  std::vector<Stmt*> stmts = comp_stmt->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) stmt->accept(this);
  }
}
void TokenVisitor::visit(v2::ForStmt *for_stmt, void *data) {
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
void TokenVisitor::visit(v2::WhileStmt *while_stmt, void *data) {
  TokenNode *token = while_stmt->getWhileNode();
  if (token) token->accept(this);
  Expr *cond = while_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
}
void TokenVisitor::visit(v2::DoStmt *do_stmt, void *data) {
  TokenNode *token = do_stmt->getDoNode();
  if (token) token->accept(this);
  Stmt *body = do_stmt->getBody();
  if (body) body->accept(this);
  token = do_stmt->getWhileNode();
  if (token) token->accept(this);
  Expr *cond = do_stmt->getCond();
  if (cond) cond->accept(this);
}
void TokenVisitor::visit(v2::BreakStmt *break_stmt, void *data) {
  IdMap[break_stmt] = id;
  Tokens.push_back(break_stmt);
  id++;
}
void TokenVisitor::visit(v2::ContinueStmt *cont_stmt, void *data) {
  IdMap[cont_stmt] = id;
  Tokens.push_back(cont_stmt);
  id++;
}
void TokenVisitor::visit(v2::ReturnStmt *ret_stmt, void *data) {
  IdMap[ret_stmt] = id;
  // Tokens.push_back(ret_stmt);
  TokenNode *ReturnNode = ret_stmt->getReturnNode();
  if (ReturnNode) ReturnNode->accept(this);
  Expr *value = ret_stmt->getValue();
  if (value) value->accept(this);
  id++;
}
void TokenVisitor::visit(v2::IfStmt *if_stmt, void *data) {
  TokenNode *token = if_stmt->getIfNode();
  if (token) token->accept(this);
  Expr *expr = if_stmt->getCond();
  if (expr) expr->accept(this);
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) then_stmt->accept(this);
  token = if_stmt->getElseNode();
  if (token) token->accept(this);
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) else_stmt->accept(this);
}
void TokenVisitor::visit(v2::SwitchStmt *switch_stmt, void *data) {
  TokenNode *token = switch_stmt->getSwitchNode();
  if (token) token->accept(this);
  Expr *cond = switch_stmt->getCond();
  if (cond) cond->accept(this);
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  for (SwitchCase *c : cases) {
    if (c) c->accept(this);
  }
}
void TokenVisitor::visit(v2::CaseStmt *case_stmt, void *data) {
  TokenNode *token = case_stmt->getCaseNode();
  if (token) token->accept(this);
  Expr *cond = case_stmt->getCond();
  if (cond) cond->accept(this);
  vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void TokenVisitor::visit(v2::DefaultStmt *def_stmt, void *data) {
  TokenNode *token = def_stmt->getDefaultNode();
  if (token) token->accept(this);
  vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void TokenVisitor::visit(v2::Expr *expr, void *data) {
  IdMap[expr] = id;
  Tokens.push_back(expr);
  id++;
}



