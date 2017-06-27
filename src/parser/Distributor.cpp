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

void Distributor::pre(v2::ASTNodeBase* node) {
  // for all the things in stack
  // push contain map
  for (ASTNodeBase* n : Stack) {
    ContainMap[n].insert(node);
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






void Distributor::dump(std::ostream &os) {
  os << "if nodes: ";
  for (auto *node : if_nodes) {
    node->dump(os);
  }
  os << "\n";
  os << "switch nodes: ";
  for (auto *node : switch_nodes) {
    node->dump(os);
  }
  os << "\n";
  os << "for nodes: ";
  for (auto *node : for_nodes) {
    node->dump(os);
  }
  os << "\n";
  os << "do nodes:";
  for (auto *node : do_nodes) {
    node->dump(os);
  }
  os << "\n";
  for (auto *node : while_nodes) {
    node->dump(os);
  }
  os << "\n";
  os << "func nodes: ";
  for (auto *node : func_nodes) {
    node->dump(os);
  }
  os << "\n";


  // containmap
  // std::map<v2::ASTNodeBase*, std::set<v2::ASTNodeBase*> > ContainMap;
  os << "Contain Map: " << "\n";
  for (auto &m : ContainMap) {
    os << "\t";
    m.first->dump(os);
    os << " ==> ";
    for (auto *node : m.second) {
      node->dump(os);
    }
    os << "\n";
  }
}
