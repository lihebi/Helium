#include "helium/parser/visitor.h"
#include "helium/parser/AST.h"
#include "helium/parser/SourceManager.h"
#include "helium/utils/StringUtils.h"
#include <iostream>

using std::vector;
using std::string;
using std::map;
using std::set;

using namespace v2;

void ParentIndexer::pre(v2::ASTNodeBase* node) {
  if (!Stack.empty()) {
    ParentMap[node] = Stack.back();
    ChildrenMap[Stack.back()].push_back(node);
  }
  Stack.push_back(node);
}
void ParentIndexer::post() {
  assert(!Stack.empty());
  Stack.pop_back();
}
// high level
void ParentIndexer::visit(v2::TokenNode *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void ParentIndexer::visit(v2::TranslationUnitDecl *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void ParentIndexer::visit(v2::FunctionDecl *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void ParentIndexer::visit(v2::CompoundStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
// condition
void ParentIndexer::visit(v2::IfStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void ParentIndexer::visit(v2::SwitchStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void ParentIndexer::visit(v2::CaseStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void ParentIndexer::visit(v2::DefaultStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
// loop
void ParentIndexer::visit(v2::ForStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void ParentIndexer::visit(v2::WhileStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void ParentIndexer::visit(v2::DoStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
// single
void ParentIndexer::visit(v2::BreakStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void ParentIndexer::visit(v2::ContinueStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void ParentIndexer::visit(v2::ReturnStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
// expr stmt
void ParentIndexer::visit(v2::Expr *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void ParentIndexer::visit(v2::DeclStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void ParentIndexer::visit(v2::ExprStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
