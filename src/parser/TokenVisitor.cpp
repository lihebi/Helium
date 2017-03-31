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


// high level
void TokenVisitor::visit(v2::TokenNode *node) {
  IdMap[node] = id;
  Tokens.push_back(node);
  id++;
  Visitor::visit(node);
}
void TokenVisitor::visit(v2::TranslationUnitDecl *node) {
  Visitor::visit(node);
}
void TokenVisitor::visit(v2::FunctionDecl *node) {
  Visitor::visit(node);
}
void TokenVisitor::visit(v2::CompoundStmt *node) {
  Visitor::visit(node);
}
// condition
void TokenVisitor::visit(v2::IfStmt *node) {
  Visitor::visit(node);
}
void TokenVisitor::visit(v2::SwitchStmt *node) {
  Visitor::visit(node);
}
void TokenVisitor::visit(v2::CaseStmt *node) {
  Visitor::visit(node);
}
void TokenVisitor::visit(v2::DefaultStmt *node) {
  Visitor::visit(node);
}
// loop
void TokenVisitor::visit(v2::ForStmt *node) {
  Visitor::visit(node);
}
void TokenVisitor::visit(v2::WhileStmt *node) {
  Visitor::visit(node);
}
void TokenVisitor::visit(v2::DoStmt *node) {
  Visitor::visit(node);
}
// single
void TokenVisitor::visit(v2::BreakStmt *node) {
  IdMap[node] = id;
  Tokens.push_back(node);
  id++;
  Visitor::visit(node);
}
void TokenVisitor::visit(v2::ContinueStmt *node) {
  IdMap[node] = id;
  Tokens.push_back(node);
  id++;
  Visitor::visit(node);
}
void TokenVisitor::visit(v2::ReturnStmt *node) {
  Visitor::visit(node);
}
// expr stmt
void TokenVisitor::visit(v2::Expr *node) {
  IdMap[node] = id;
  Tokens.push_back(node);
  id++;
  Visitor::visit(node);
}
void TokenVisitor::visit(v2::DeclStmt *node) {
  IdMap[node] = id;
  Tokens.push_back(node);
  id++;
  Visitor::visit(node);
}
void TokenVisitor::visit(v2::ExprStmt *node) {
  IdMap[node] = id;
  Tokens.push_back(node);
  id++;
  Visitor::visit(node);
}
