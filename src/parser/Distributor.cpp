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

void Distributor::pre(v2::ASTNodeBase* node) {
  // for all the things in stack
  // push contain map
  for (ASTNodeBase* node : Stack) {
    ContainMap[node].insert(node);
  }
  Stack.push_back(node);
}

void Distributor::post() {
  Stack.pop_back();
}

// high level
void Distributor::visit(v2::TokenNode *node){
  pre(node);
  Visitor::visit(node);
  post();
}
void Distributor::visit(v2::TranslationUnitDecl *node){
  pre(node);
  Visitor::visit(node);
  post();
}
void Distributor::visit(v2::FunctionDecl *node){
  func_nodes.insert(node);
  pre(node);
  Visitor::visit(node);
  post();
}
void Distributor::visit(v2::CompoundStmt *node){
  pre(node);
  Visitor::visit(node);
  post();
}
// condition
void Distributor::visit(v2::IfStmt *node){
  if_nodes.insert(node);
  pre(node);
  Visitor::visit(node);
  post();
}
void Distributor::visit(v2::SwitchStmt *node){
  switch_nodes.insert(node);
  pre(node);
  Visitor::visit(node);
  post();
}
void Distributor::visit(v2::CaseStmt *node){
  pre(node);
  Visitor::visit(node);
  post();
}
void Distributor::visit(v2::DefaultStmt *node){
  pre(node);
  Visitor::visit(node);
  post();
}
// loop
void Distributor::visit(v2::ForStmt *node){
  for_nodes.insert(node);
  pre(node);
  Visitor::visit(node);
  post();
}
void Distributor::visit(v2::WhileStmt *node){
  while_nodes.insert(node);
  pre(node);
  Visitor::visit(node);
  post();
}
void Distributor::visit(v2::DoStmt *node){
  do_nodes.insert(node);
  pre(node);
  Visitor::visit(node);
  post();
}
// single
void Distributor::visit(v2::BreakStmt *node){
  pre(node);
  Visitor::visit(node);
  post();
}
void Distributor::visit(v2::ContinueStmt *node){
  pre(node);
  Visitor::visit(node);
  post();
}
void Distributor::visit(v2::ReturnStmt *node){
  pre(node);
  Visitor::visit(node);
  post();
}
// expr stmt
void Distributor::visit(v2::Expr *node){
  pre(node);
  Visitor::visit(node);
  post();
}
void Distributor::visit(v2::DeclStmt *node){
  pre(node);
  Visitor::visit(node);
  post();
}
void Distributor::visit(v2::ExprStmt *node){
  pre(node);
  Visitor::visit(node);
  post();
}
