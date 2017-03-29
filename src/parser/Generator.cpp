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

// TODO
void Generator::visit(v2::TokenNode *token, void *data) {
  if (selection.count(token) == 1) {
    Prog += token->getText() + " ";
  }
}
void Generator::visit(v2::TranslationUnitDecl *unit, void *data) {
  std::vector<ASTNodeBase*> nodes = unit->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) node->accept(this);
  }
}
void Generator::visit(v2::FunctionDecl *function, void *data) {
  TokenNode *ReturnNode = function->getReturnTypeNode();
  if (ReturnNode) ReturnNode->accept(this);
  Prog += " ";
  TokenNode *NameNode = function->getNameNode();
  if (NameNode) NameNode->accept(this);
  // param node should handle parenthesis
  TokenNode *ParamNode = function->getParamNode();
  if (ParamNode) ParamNode->accept(this);
  // compound should handle curly braces
  Stmt *body = function->getBody();
  if (body) body->accept(this);
}
void Generator::visit(v2::DeclStmt *decl_stmt, void *data) {
  if (selection.count(decl_stmt) == 1) {
    Prog += decl_stmt->getText() + "\n";
  }
}
void Generator::visit(v2::ExprStmt *expr_stmt, void *data) {
  if (selection.count(expr_stmt) == 1) {
    // no need semi-colon because <expr_stmt>... ;</expr_stmt>
    Prog += expr_stmt->getText() + "\n";
  }
}
void Generator::visit(v2::CompoundStmt *comp_stmt, void *data) {
  // Braces
  TokenNode *CompNode = comp_stmt->getCompNode();
  if (selection.count(CompNode) == 1) {Prog += "{\n";}
  std::vector<Stmt*> stmts = comp_stmt->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) stmt->accept(this);
  }
  if (selection.count(CompNode) == 1) {Prog += "}\n";}
}
void Generator::visit(v2::ForStmt *for_stmt, void *data) {
  TokenNode *ForNode = for_stmt->getForNode();
  assert(ForNode);
  ForNode->accept(this);
  if (selection.count(ForNode) == 1) Prog += "(";
  Expr *init = for_stmt->getInit();
  if (init) init->accept(this);
  if (selection.count(ForNode) == 1) Prog += ";";
  Expr *cond = for_stmt->getCond();
  if (cond) cond->accept(this);
  if (selection.count(ForNode) == 1) Prog += ";";
  Expr *inc = for_stmt->getInc();
  if (inc) inc->accept(this);
  if (selection.count(ForNode) == 1) Prog += ")";
  Stmt *body = for_stmt->getBody();
  if (body) body->accept(this);
}
void Generator::visit(v2::WhileStmt *while_stmt, void *data) {
  TokenNode *WhileNode = while_stmt->getWhileNode();
  if (WhileNode) WhileNode->accept(this);
  if (selection.count(WhileNode) == 1) Prog += "(";
  Expr *cond = while_stmt->getCond();
  if (cond) cond->accept(this);
  if (selection.count(WhileNode) == 1) Prog += ")";
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
}
void Generator::visit(v2::DoStmt *do_stmt, void *data) {
  TokenNode *DoNode = do_stmt->getDoNode();
  TokenNode *WhileNode = do_stmt->getWhileNode();
  assert(DoNode);
  assert(WhileNode);
  DoNode->accept(this);
  Stmt *body = do_stmt->getBody();
  if (body) body->accept(this);
  WhileNode->accept(this);
  if (selection.count(WhileNode) == 1) {Prog += "(";}
  Expr *cond = do_stmt->getCond();
  if (cond) cond->accept(this);
  if (selection.count(WhileNode) == 1) {Prog += ");";}
}
void Generator::visit(v2::BreakStmt *break_stmt, void *data) {
  if (selection.count(break_stmt)) {
    Prog += "break;\n";
  }
}
void Generator::visit(v2::ContinueStmt *cont_stmt, void *data) {
  if (selection.count(cont_stmt)) {
    Prog += "continue;\n";
  }
}
void Generator::visit(v2::ReturnStmt *ret_stmt, void *data) {
  TokenNode *ReturnNode = ret_stmt->getReturnNode();
  if (ReturnNode) {
    ReturnNode->accept(this);
  }
  Expr *expr = ret_stmt->getValue();
  if (expr) expr->accept(this);
  if (selection.count(ReturnNode) == 1) {
    Prog += ";\n";
  }
}
void Generator::visit(v2::IfStmt *if_stmt, void *data) {
  TokenNode *IfNode = if_stmt->getIfNode();
  assert(IfNode);
  IfNode->accept(this);
  if (selection.count(IfNode)==1) Prog += "(";
  Expr *expr = if_stmt->getCond();
  if (expr) expr->accept(this);
  if (selection.count(IfNode)==1) Prog += ")";

  // if (selection.count(IfNode) == 1) {Prog += "{";}
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) then_stmt->accept(this);
  // if (selection.count(IfNode) == 1) {Prog += "}";}

  TokenNode *ElseNode = if_stmt->getElseNode();
  if (ElseNode) ElseNode->accept(this);
  // if (selection.count(ElseNode) == 1) {Prog += "{";}
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) else_stmt->accept(this);
  // if (selection.count(ElseNode) == 1) {Prog += "}";}
}
void Generator::visit(v2::SwitchStmt *switch_stmt, void *data) {
  TokenNode *SwitchNode = switch_stmt->getSwitchNode();
  assert(SwitchNode);
  SwitchNode->accept(this);
  if (selection.count(SwitchNode) == 1) Prog += "(";
  Expr *cond = switch_stmt->getCond();
  if (cond) cond->accept(this);
  if (selection.count(SwitchNode) == 1) Prog += ")";
  if (selection.count(SwitchNode) == 1) Prog += "{// switch";
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  for (SwitchCase *c : cases) {
    if (c) c->accept(this);
  }
  if (selection.count(SwitchNode) == 1) Prog += "}";
}
void Generator::visit(v2::CaseStmt *case_stmt, void *data) {
  TokenNode *token = case_stmt->getCaseNode();
  if (token) token->accept(this);
  Expr *cond = case_stmt->getCond();
  if (cond) cond->accept(this);
  vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void Generator::visit(v2::DefaultStmt *def_stmt, void *data) {
  TokenNode *token = def_stmt->getDefaultNode();
  if (token) token->accept(this);
  vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void Generator::visit(v2::Expr *expr, void *data) {
  if (selection.count(expr) == 1) {
    Prog += expr->getText();
  }
}
