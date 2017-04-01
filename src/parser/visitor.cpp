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
  // token node
  comp_stmt->getCompNode()->accept(this);
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


// high level
void SimplePreorderVisitor::visit(v2::TokenNode *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
void SimplePreorderVisitor::visit(v2::TranslationUnitDecl *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
void SimplePreorderVisitor::visit(v2::FunctionDecl *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
void SimplePreorderVisitor::visit(v2::CompoundStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
// condition
void SimplePreorderVisitor::visit(v2::IfStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
void SimplePreorderVisitor::visit(v2::SwitchStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
void SimplePreorderVisitor::visit(v2::CaseStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
void SimplePreorderVisitor::visit(v2::DefaultStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
// loop
void SimplePreorderVisitor::visit(v2::ForStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
void SimplePreorderVisitor::visit(v2::WhileStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
void SimplePreorderVisitor::visit(v2::DoStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
// single
void SimplePreorderVisitor::visit(v2::BreakStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
void SimplePreorderVisitor::visit(v2::ContinueStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
void SimplePreorderVisitor::visit(v2::ReturnStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
// expr stmt
void SimplePreorderVisitor::visit(v2::Expr *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
void SimplePreorderVisitor::visit(v2::DeclStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}
void SimplePreorderVisitor::visit(v2::ExprStmt *node) {
  Nodes.push_back(node);
  Visitor::visit(node);
}


void Matcher::pre(v2::ASTNodeBase* node) {
  std::string name = node->getNodeName();
  Stack.push_back(name);
  // push to map
  PathMap[currentName()].push_back(node);

  // nodes
  Nodes.push_back(node);
}
void Matcher::post() {
  Stack.pop_back();
}

// high level
void Matcher::visit(v2::TokenNode *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Matcher::visit(v2::TranslationUnitDecl *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Matcher::visit(v2::FunctionDecl *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Matcher::visit(v2::CompoundStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
// condition
void Matcher::visit(v2::IfStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Matcher::visit(v2::SwitchStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Matcher::visit(v2::CaseStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Matcher::visit(v2::DefaultStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
// loop
void Matcher::visit(v2::ForStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Matcher::visit(v2::WhileStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Matcher::visit(v2::DoStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
// single
void Matcher::visit(v2::BreakStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Matcher::visit(v2::ContinueStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Matcher::visit(v2::ReturnStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
// expr stmt
void Matcher::visit(v2::Expr *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Matcher::visit(v2::DeclStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Matcher::visit(v2::ExprStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}



v2::ASTNodeBase* Matcher::getNodeByLoc(std::string name, int line) {
  for (auto *node : Nodes) {
    SourceLocation begin = node->getBeginLoc();
    if (node->getNodeName() == name && begin.getLine() == line) {
      return node;
    }
  }
  return nullptr;
}
v2::ASTNodeBase* Matcher::getNodeByLoc(std::string name, int line, int nth) {
  for (auto *node : Nodes) {
    SourceLocation begin = node->getBeginLoc();
    if (node->getNodeName() == name && begin.getLine() == line) {
      if (nth--<=0) {
        return node;
      }
    }
  }
  return nullptr;
}


v2::ASTNodeBase* Matcher::getNodeByLoc(std::string name, SourceLocation loc) {
  for (auto *node : Nodes) {
    SourceLocation begin = node->getBeginLoc();
    if (node->getNodeName() == name && begin == loc) {
      return node;
    }
  }
  return nullptr;
}
v2::ASTNodeBase* Matcher::getNodeByLoc(std::string name, SourceLocation loc, int nth) {
  for (auto *node : Nodes) {
    SourceLocation begin = node->getBeginLoc();
    if (node->getNodeName() == name && begin == loc) {
      if (nth--<=0) {
        return node;
      }
    }
  }
  return nullptr;
}
