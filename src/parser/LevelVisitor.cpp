#include "helium/parser/visitor.h"
#include "helium/parser/AST.h"
#include "helium/parser/SourceManager.h"
#include "helium/utils/StringUtils.h"
#include <iostream>

#include <gtest/gtest.h>

using std::vector;
using std::string;
using std::map;
using std::set;

using namespace v2;




// level visitor
void LevelVisitor::visit(v2::TokenNode *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void LevelVisitor::visit(v2::TranslationUnitDecl *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void LevelVisitor::visit(v2::FunctionDecl *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void LevelVisitor::visit(v2::CompoundStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
// condition
void LevelVisitor::visit(v2::IfStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void LevelVisitor::visit(v2::SwitchStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void LevelVisitor::visit(v2::CaseStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void LevelVisitor::visit(v2::DefaultStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
// loop
void LevelVisitor::visit(v2::ForStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void LevelVisitor::visit(v2::WhileStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void LevelVisitor::visit(v2::DoStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
// single
void LevelVisitor::visit(v2::BreakStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void LevelVisitor::visit(v2::ContinueStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void LevelVisitor::visit(v2::ReturnStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
// expr stmt
void LevelVisitor::visit(v2::Expr *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void LevelVisitor::visit(v2::DeclStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void LevelVisitor::visit(v2::ExprStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}


int LevelVisitor::getLevel(v2::ASTNodeBase *node) {
  if (Levels.count(node) == 1) {
    return Levels[node];
  } else {
    return -1;
  }
}


v2::ASTNodeBase *LevelVisitor::getLowestLevelNode(std::set<v2::ASTNodeBase*> nodes) {
  int retlvl=-1;
  v2::ASTNodeBase *ret = nullptr;
  for (auto *node : nodes) {
    int lvl = getLevel(node);
    if (retlvl < lvl) {
      retlvl = lvl;
      ret = node;
    }
  }
  return ret;
}
