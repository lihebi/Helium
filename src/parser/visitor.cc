#include "helium/parser/visitor.h"
#include "helium/parser/ast_v2.h"
#include "helium/parser/source_manager.h"

#include <iostream>

using namespace v2;

void LevelVisitor::visit(v2::TranslationUnitDecl *unit) {
  Levels[unit] = lvl;
  std::vector<ASTNodeBase*> decls = unit->getDecls();
  lvl++;
  for (ASTNodeBase *decl : decls) {
    // this->visit(decl);
    decl->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::FunctionDecl *function) {
  Levels[function] = lvl;
  Stmt *body = function->getBody();
  lvl++;
  // this->visit(body);
  body->accept(this);
  lvl--;
}

void LevelVisitor::visit(v2::DeclStmt *decl_stmt) {
  Levels[decl_stmt] = lvl;
}
void LevelVisitor::visit(v2::ExprStmt *expr_stmt) {
  Levels[expr_stmt] = lvl;
}
void LevelVisitor::visit(v2::CompoundStmt *comp_stmt) {
  Levels[comp_stmt] = lvl;
  std::vector<Stmt*> body = comp_stmt->getBody();
  lvl++;
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::ForStmt *for_stmt) {
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
void LevelVisitor::visit(v2::WhileStmt *while_stmt) {
  Levels[while_stmt] = lvl;
  lvl++;
  Expr *cond = while_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
  lvl--;
}
void LevelVisitor::visit(v2::DoStmt *do_stmt) {
  Levels[do_stmt] = lvl;
  lvl++;
  Expr *cond = do_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *stmt = do_stmt->getBody();
  if (stmt) stmt->accept(this);
  lvl--;
}
void LevelVisitor::visit(v2::BreakStmt *break_stmt) {
  Levels[break_stmt] = lvl;
}
void LevelVisitor::visit(v2::ContinueStmt *cont_stmt) {
  Levels[cont_stmt] = lvl;
}
void LevelVisitor::visit(v2::ReturnStmt *ret_stmt) {
  Levels[ret_stmt] = lvl;
}
void LevelVisitor::visit(v2::IfStmt *if_stmt) {
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
void LevelVisitor::visit(v2::SwitchStmt *switch_stmt) {
  Levels[switch_stmt] = lvl;
  lvl++;
  Expr *cond = switch_stmt->getCond();
  if (cond) cond->accept(this);
  switch_stmt->getCases();
  lvl--;
}

void LevelVisitor::visit(v2::CaseStmt *case_stmt) {
  Levels[case_stmt] = lvl;
  lvl++;
  std::vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    stmt->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::DefaultStmt *def_stmt) {
  Levels[def_stmt] = lvl;
  lvl++;
  std::vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    stmt->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::Expr *expr) {
  Levels[expr] = lvl;
}













void Printer::visit(TranslationUnitDecl *unit) {
  os << "unit";
  std::vector<ASTNodeBase*> nodes = unit->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) node->accept(this);
  }
}
void Printer::visit(FunctionDecl *function){
  os << "function";
  // ASTContext *ast = function->getASTContext();
  // int id = -1;
  // if (ast) {
  //   id = ast->getSourceManager()->getIdByNode(function);
  // }
  // int id = function->getASTContext()->getSourceManager()->getIdByNode(function);
  std::string name = function->getName();
  os << name;
  // os << id << ":" << name;
  Stmt *body = function->getBody();
  body->accept(this);
}
void Printer::visit(DeclStmt *decl_stmt){
  os << "decl_stmt";
}
void Printer::visit(ExprStmt *expr_stmt){
  os << "expr_stmt";
}
void Printer::visit(CompoundStmt *comp_stmt){
  os << "comp_stmt";
}
void Printer::visit(ForStmt *for_stmt){
  os << "for_stmt";
}
void Printer::visit(WhileStmt *while_stmt){
  os << "while_stmt";
}
void Printer::visit(DoStmt *do_stmt){
  os << "do_stmt";
}
void Printer::visit(BreakStmt *break_stmt){
  os << "break_stmt";
}
void Printer::visit(ContinueStmt *cont_stmt){
  os << "cont_stmt";
}
void Printer::visit(ReturnStmt *ret_stmt){
  os << "ret_stmt";
}
void Printer::visit(IfStmt *if_stmt){
  os << "if_stmt";
}
void Printer::visit(SwitchStmt *switch_stmt){
  os << "switch_stmt";
}
void Printer::visit(CaseStmt *case_stmt){
  os << "case_stmt";
}
void Printer::visit(DefaultStmt *def_stmt){
  os << "def_stmt";
}
void Printer::visit(Expr *expr){
  os << "expr_stmt";
}
