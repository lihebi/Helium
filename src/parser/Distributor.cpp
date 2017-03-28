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



// std::map<v2::ASTNodeBase*, std::set<v2::ASTNodeBase*> > ContainMap;
// std::set<v2::ASTNodeBase*> if_nodes;
// std::set<v2::ASTNodeBase*> switch_nodes;
// std::set<v2::ASTNodeBase*> for_nodes;
// std::set<v2::ASTNodeBase*> do_nodes;
// std::set<v2::ASTNodeBase*> while_nodes;
// std::set<v2::ASTNodeBase*> func_nodes;

void Distributor::visit(v2::TokenNode *token, void *data) {}
void Distributor::visit(v2::TranslationUnitDecl *unit, void *data) {
  std::vector<ASTNodeBase*> nodes = unit->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) {
      node->accept(this);
      addTo(node, unit);
    }
  }
}
void Distributor::visit(v2::FunctionDecl *function, void *data) {
  TokenNode *ReturnNode = function->getReturnTypeNode();
  if (ReturnNode) ContainMap[function].insert(ReturnNode);
  TokenNode *NameNode = function->getNameNode();
  if (NameNode) {
    ContainMap[function].insert(NameNode);
  }
  TokenNode *ParamNode = function->getParamNode();
  if (ParamNode) {
    ContainMap[function].insert(ParamNode);
  }
  Stmt *body = function->getBody();
  if (body) {
    body->accept(this);
    // body is already processed, thus the map for it is useful
    addTo(body, function);
  }
  func_nodes.insert(function);
}
void Distributor::visit(v2::DeclStmt *decl_stmt, void *data) {
  // FIXME
  // ContainMap[decl_stmt].insert(decl_stmt);
}
void Distributor::visit(v2::ExprStmt *expr_stmt, void *data) {}
void Distributor::visit(v2::CompoundStmt *comp_stmt, void *data) {
  std::vector<Stmt*> stmts = comp_stmt->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) {
      stmt->accept(this);
      addTo(stmt, comp_stmt);
    }
  }
}
void Distributor::visit(v2::ForStmt *for_stmt, void *data) {
  TokenNode *token = for_stmt->getForNode();
  if (token) {
    ContainMap[for_stmt].insert(token);
  }
  Expr *init = for_stmt->getInit();
  if (init) {
    ContainMap[for_stmt].insert(init);
  }
  Expr *cond = for_stmt->getCond();
  if (cond) {
    ContainMap[for_stmt].insert(cond);
  }
  Expr *inc = for_stmt->getInc();
  if (inc) ContainMap[for_stmt].insert(inc);
  Stmt *body = for_stmt->getBody();
  if (body) {
    body->accept(this);
    addTo(body, for_stmt);
  }

  for_nodes.insert(for_stmt);
}
void Distributor::visit(v2::WhileStmt *while_stmt, void *data) {
  TokenNode *token = while_stmt->getWhileNode();
  if (token) ContainMap[while_stmt].insert(token);
  Expr *cond = while_stmt->getCond();
  if (cond) ContainMap[while_stmt].insert(cond);
  Stmt *body = while_stmt->getBody();
  if (body) {
    body->accept(this);
    addTo(body, while_stmt);
  }

  while_nodes.insert(while_stmt);
}
void Distributor::visit(v2::DoStmt *do_stmt, void *data) {
  TokenNode *token = do_stmt->getDoNode();
  if (token) ContainMap[do_stmt].insert(token);
  Stmt *body = do_stmt->getBody();
  if (body) {
    body->accept(this);
    addTo(body, do_stmt);
  }
  token = do_stmt->getWhileNode();
  if (token) ContainMap[do_stmt].insert(token);
  Expr *cond = do_stmt->getCond();
  if (cond) ContainMap[do_stmt].insert(cond);

  do_nodes.insert(do_stmt);
}
void Distributor::visit(v2::BreakStmt *break_stmt, void *data) {}
void Distributor::visit(v2::ContinueStmt *cont_stmt, void *data) {}
void Distributor::visit(v2::ReturnStmt *ret_stmt, void *data) {
  TokenNode *ReturnNode = ret_stmt->getReturnNode();
  if (ReturnNode) ContainMap[ret_stmt].insert(ReturnNode);
  Expr *value = ret_stmt->getValue();
  if (value) ContainMap[ret_stmt].insert(ReturnNode);
}
void Distributor::visit(v2::IfStmt *if_stmt, void *data) {
  TokenNode *token = if_stmt->getIfNode();
  if (token) ContainMap[if_stmt].insert(token);
  Expr *expr = if_stmt->getCond();
  if (expr) ContainMap[if_stmt].insert(expr);
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) {
    then_stmt->accept(this);
    addTo(then_stmt, if_stmt);
  }
  token = if_stmt->getElseNode();
  if (token) ContainMap[if_stmt].insert(token);
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) {
    else_stmt->accept(this);
    addTo(else_stmt, if_stmt);
  }

  // add to if_nodes
  if_nodes.insert(if_stmt);
}
void Distributor::visit(v2::SwitchStmt *switch_stmt, void *data) {
  TokenNode *token = switch_stmt->getSwitchNode();
  if (token) ContainMap[switch_stmt].insert(token);
  Expr *cond = switch_stmt->getCond();
  if (cond) ContainMap[switch_stmt].insert(cond);
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  for (SwitchCase *c : cases) {
    if (c) {
      c->accept(this);
      addTo(c, switch_stmt);
    }
  }

  switch_nodes.insert(switch_stmt);
}
void Distributor::visit(v2::CaseStmt *case_stmt, void *data) {
  TokenNode *token = case_stmt->getCaseNode();
  if (token) ContainMap[case_stmt].insert(token);
  Expr *cond = case_stmt->getCond();
  if (cond) ContainMap[case_stmt].insert(cond);
  vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) {
      stmt->accept(this);
      addTo(stmt, case_stmt);
    }
  }
}
void Distributor::visit(v2::DefaultStmt *def_stmt, void *data) {
  TokenNode *token = def_stmt->getDefaultNode();
  if (token) ContainMap[def_stmt].insert(token);
  vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) {
      stmt->accept(this);
      addTo(stmt, def_stmt);
    }
  }
}
void Distributor::visit(v2::Expr *expr, void *data) {}

