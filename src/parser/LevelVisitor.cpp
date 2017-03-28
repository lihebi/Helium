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



void LevelVisitor::visit(v2::TokenNode *token, void *data) {
  Levels[token] = lvl;
  return;
}

void LevelVisitor::visit(v2::TranslationUnitDecl *unit, void *data) {
  Levels[unit] = lvl;
  std::vector<ASTNodeBase*> decls = unit->getDecls();
  lvl++;
  for (ASTNodeBase *decl : decls) {
    // this->visit(decl);
    decl->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::FunctionDecl *function, void *data) {
  Levels[function] = lvl;
  Stmt *body = function->getBody();
  lvl++;
  // this->visit(body);
  body->accept(this);
  lvl--;
}

void LevelVisitor::visit(v2::DeclStmt *decl_stmt, void *data) {
  Levels[decl_stmt] = lvl;
}
void LevelVisitor::visit(v2::ExprStmt *expr_stmt, void *data) {
  Levels[expr_stmt] = lvl;
}
void LevelVisitor::visit(v2::CompoundStmt *comp_stmt, void *data) {
  Levels[comp_stmt] = lvl;
  std::vector<Stmt*> body = comp_stmt->getBody();
  lvl++;
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::ForStmt *for_stmt, void *data) {
  Levels[for_stmt] = lvl;
  lvl++;
  Expr *init = for_stmt->getInit();
  if (init) init->accept(this);
  Expr *cond = for_stmt->getCond();
  if (cond) cond->accept(this);
  Expr *inc = for_stmt->getInc();
  if (inc) inc->accept(this);
  Stmt *body = for_stmt->getBody();
  if (body) body->accept(this);
  lvl--;
}
void LevelVisitor::visit(v2::WhileStmt *while_stmt, void *data) {
  Levels[while_stmt] = lvl;
  lvl++;
  Expr *cond = while_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
  lvl--;
}
void LevelVisitor::visit(v2::DoStmt *do_stmt, void *data) {
  Levels[do_stmt] = lvl;
  lvl++;
  Expr *cond = do_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *stmt = do_stmt->getBody();
  if (stmt) stmt->accept(this);
  lvl--;
}
void LevelVisitor::visit(v2::BreakStmt *break_stmt, void *data) {
  Levels[break_stmt] = lvl;
}
void LevelVisitor::visit(v2::ContinueStmt *cont_stmt, void *data) {
  Levels[cont_stmt] = lvl;
}
void LevelVisitor::visit(v2::ReturnStmt *ret_stmt, void *data) {
  Levels[ret_stmt] = lvl;
}
void LevelVisitor::visit(v2::IfStmt *if_stmt, void *data) {
  Levels[if_stmt] = lvl;
  lvl++;
  Expr *cond = if_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) then_stmt->accept(this);
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) else_stmt->accept(this);
  lvl--;
}
void LevelVisitor::visit(v2::SwitchStmt *switch_stmt, void *data) {
  Levels[switch_stmt] = lvl;
  lvl++;
  Expr *cond = switch_stmt->getCond();
  if (cond) cond->accept(this);
  switch_stmt->getCases();
  lvl--;
}

void LevelVisitor::visit(v2::CaseStmt *case_stmt, void *data) {
  Levels[case_stmt] = lvl;
  lvl++;
  std::vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    stmt->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::DefaultStmt *def_stmt, void *data) {
  Levels[def_stmt] = lvl;
  lvl++;
  std::vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    stmt->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::Expr *expr, void *data) {
  Levels[expr] = lvl;
}


