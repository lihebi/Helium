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


void ParentIndexer::visit(v2::TokenNode *token, void *data) {}
void ParentIndexer::visit(v2::TranslationUnitDecl *unit, void *data) {
  std::vector<ASTNodeBase*> nodes = unit->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) {
      ParentMap[node] = unit;
      ChildrenMap[unit].insert(node);
      node->accept(this);
    }
  }
}
void ParentIndexer::visit(v2::FunctionDecl *function, void *data) {
  TokenNode *token = function->getReturnTypeNode();
  if (token) {
    ParentMap[token] = function;
    ChildrenMap[function].insert(token);
    token->accept(this);
  }
  token = function->getNameNode();
  if (token) {
    ParentMap[token] = function;
    ChildrenMap[function].insert(token);
    token->accept(this);
  }
  token = function->getParamNode();
  if (token) {
    ParentMap[token] = function;
    ChildrenMap[function].insert(token);
    token->accept(this);
  }
  Stmt *body = function->getBody();
  if (body) {
    ParentMap[body] = function;
    ChildrenMap[function].insert(body);
    body->accept(this);
  }
}
void ParentIndexer::visit(v2::DeclStmt *decl_stmt, void *data) {}
void ParentIndexer::visit(v2::ExprStmt *expr_stmt, void *data) {}
void ParentIndexer::visit(v2::CompoundStmt *comp_stmt, void *data) {
  std::vector<Stmt*> stmts = comp_stmt->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) {
      ParentMap[stmt] = comp_stmt;
      ChildrenMap[comp_stmt].insert(stmt);
      stmt->accept(this);
    }
  }
}
void ParentIndexer::visit(v2::ForStmt *for_stmt, void *data) {
  TokenNode *token = for_stmt->getForNode();
  if (token) {
    ParentMap[token] = for_stmt;
    ChildrenMap[for_stmt].insert(token);
    token->accept(this);
  }
  Expr *init = for_stmt->getInit();
  if (init) {
    ParentMap[init] = for_stmt;
    ChildrenMap[for_stmt].insert(init);
    init->accept(this);
  }
  Expr *cond = for_stmt->getCond();
  if (cond) {
    ParentMap[cond] = for_stmt;
    ChildrenMap[for_stmt].insert(cond);
    cond->accept(this);
  }
  Expr *inc = for_stmt->getInc();
  if (inc) {
    ParentMap[inc] = for_stmt;
    ChildrenMap[for_stmt].insert(inc);
    inc->accept(this);
  }
  Stmt *body = for_stmt->getBody();
  if (body) {
    ParentMap[body] = for_stmt;
    ChildrenMap[for_stmt].insert(body);
    body->accept(this);
  }
}
void ParentIndexer::visit(v2::WhileStmt *while_stmt, void *data) {
  TokenNode *token = while_stmt->getWhileNode();
  if (token) {
    ParentMap[token] = while_stmt;
    ChildrenMap[while_stmt].insert(token);
    token->accept(this);
  }
  Expr *cond = while_stmt->getCond();
  if (cond) {
    ParentMap[cond] = while_stmt;
    ChildrenMap[while_stmt].insert(cond);
    cond->accept(this);
  }
  Stmt *body = while_stmt->getBody();
  if (body) {
    ParentMap[body] = while_stmt;
    ChildrenMap[while_stmt].insert(body);
    body->accept(this);
  }
}
void ParentIndexer::visit(v2::DoStmt *do_stmt, void *data) {
  TokenNode *token = do_stmt->getDoNode();
  if (token) {
    ParentMap[token] = do_stmt;
    ChildrenMap[do_stmt].insert(token);
    token->accept(this);
  }
  Stmt *body = do_stmt->getBody();
  if (body) {
    ParentMap[body] = do_stmt;
    ChildrenMap[do_stmt].insert(body);
    body->accept(this);
  }
  token = do_stmt->getWhileNode();
  if (token) {
    ParentMap[token] = do_stmt;
    ChildrenMap[do_stmt].insert(token);
    token->accept(this);
  }
  Expr *cond = do_stmt->getCond();
  if (cond) {
    ParentMap[token] = do_stmt;
    ChildrenMap[do_stmt].insert(token);
    cond->accept(this);
  }
}
void ParentIndexer::visit(v2::BreakStmt *break_stmt, void *data) {}
void ParentIndexer::visit(v2::ContinueStmt *cont_stmt, void *data) {}
void ParentIndexer::visit(v2::ReturnStmt *ret_stmt, void *data) {
  TokenNode *ReturnNode = ret_stmt->getReturnNode();
  if (ReturnNode) {
    ParentMap[ReturnNode] = ret_stmt;
    ChildrenMap[ret_stmt].insert(ReturnNode);
    ReturnNode->accept(this);
  }
  Expr *value = ret_stmt->getValue();
  if (value) {
    ParentMap[value] = ret_stmt;
    ChildrenMap[ret_stmt].insert(value);
    value->accept(this);
  }
}
void ParentIndexer::visit(v2::IfStmt *if_stmt, void *data) {
  TokenNode *token = if_stmt->getIfNode();
  if (token) {
    ParentMap[token] = if_stmt;
    ChildrenMap[if_stmt].insert(token);
    token->accept(this);
  }
  Expr *expr = if_stmt->getCond();
  if (expr) {
    ParentMap[expr] = if_stmt;
    ChildrenMap[if_stmt].insert(expr);
    expr->accept(this);
  }
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) {
    ParentMap[then_stmt] = if_stmt;
    ChildrenMap[if_stmt].insert(then_stmt);
    then_stmt->accept(this);
  }
  token = if_stmt->getElseNode();
  if (token) {
    ParentMap[token] = if_stmt;
    ChildrenMap[if_stmt].insert(token);
    token->accept(this);
  }
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) {
    ParentMap[else_stmt] = if_stmt;
    ChildrenMap[if_stmt].insert(else_stmt);
    else_stmt->accept(this);
  }
}
void ParentIndexer::visit(v2::SwitchStmt *switch_stmt, void *data) {
  TokenNode *token = switch_stmt->getSwitchNode();
  if (token) {
    ParentMap[token] = switch_stmt;
    ChildrenMap[switch_stmt].insert(token);
    token->accept(this);
  }
  Expr *cond = switch_stmt->getCond();
  if (cond) {
    ParentMap[cond] = switch_stmt;
    ChildrenMap[switch_stmt].insert(cond);
    cond->accept(this);
  }
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  for (SwitchCase *c : cases) {
    if (c) {
      ParentMap[c] = switch_stmt;
      ChildrenMap[switch_stmt].insert(c);
      c->accept(this);
    }
  }
}
void ParentIndexer::visit(v2::CaseStmt *case_stmt, void *data) {
  TokenNode *token = case_stmt->getCaseNode();
  if (token) {
    ParentMap[token] = case_stmt;
    ChildrenMap[case_stmt].insert(token);
    token->accept(this);
  }
  Expr *cond = case_stmt->getCond();
  if (cond) {
    ParentMap[cond] = case_stmt;
    ChildrenMap[case_stmt].insert(cond);
    cond->accept(this);
  }
  vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) {
      ParentMap[stmt] = case_stmt;
      ChildrenMap[case_stmt].insert(stmt);
      stmt->accept(this);
    }
  }
}
void ParentIndexer::visit(v2::DefaultStmt *def_stmt, void *data) {
  TokenNode *token = def_stmt->getDefaultNode();
  if (token) {
    ParentMap[token] = def_stmt;
    ChildrenMap[def_stmt].insert(token);
    token->accept(this);
  }
  vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) {
      ParentMap[stmt] = def_stmt;
      ChildrenMap[def_stmt].insert(stmt);
      stmt->accept(this);
    }
  }
}
void ParentIndexer::visit(v2::Expr *expr, void *data) {}

