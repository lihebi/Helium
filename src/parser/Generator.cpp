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
void Generator::visit(v2::TokenNode *node){
  if (selection.count(node) == 1) {
    Prog += node->getText() + " ";
  }
}
void Generator::visit(v2::TranslationUnitDecl *node){
  std::vector<ASTNodeBase*> nodes = node->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) node->accept(this);
  }
}
void Generator::visit(v2::FunctionDecl *node){
  TokenNode *ReturnNode = node->getReturnTypeNode();
  if (ReturnNode) ReturnNode->accept(this);
  Prog += " ";

  // TRICK: do a trick here: If the function is main, replace the name
  // to helium_main there must be only main function right.. unless
  // the code contains many exectuables .. That is not interesting

  TokenNode *NameNode = node->getNameNode();
  if (NameNode) {
    // NameNode->accept(this);
    if (selection.count(NameNode) == 1) {
      // only if this function is selected will do the trick,
      // otherwise I will get a lot of helium_main
      if (NameNode->getText() == "main") {
        Prog += "helium_main";
      } else {
        NameNode->accept(this);
      }
    }
  }
  // param node should handle parenthesis
  TokenNode *ParamNode = node->getParamNode();
  if (ParamNode) ParamNode->accept(this);
  // compound should handle curly braces
  Stmt *body = node->getBody();
  if (body) body->accept(this);
}
void Generator::visit(v2::CompoundStmt *node){
  // Braces
  TokenNode *CompNode = node->getCompNode();
  if (selection.count(CompNode) == 1) {Prog += "{\n";}
  std::vector<Stmt*> stmts = node->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) stmt->accept(this);
  }
  if (selection.count(CompNode) == 1) {Prog += "}\n";}
}
// condition
void Generator::visit(v2::IfStmt *node){
  TokenNode *IfNode = node->getIfNode();
  assert(IfNode);
  if (selection.count(IfNode)==1) {
    Prog += "// " + node->getASTContext()->getFileName() + ":"
      + std::to_string(node->getBeginLoc().getLine()) + "\n";
  }
  IfNode->accept(this);
  if (selection.count(IfNode)==1) Prog += "(";
  Expr *expr = node->getCond();
  if (expr) expr->accept(this);
  if (selection.count(IfNode)==1) Prog += ")";

  // if (selection.count(IfNode) == 1) {Prog += "{";}
  Stmt *then_stmt = node->getThen();
  if (then_stmt) then_stmt->accept(this);
  // if (selection.count(IfNode) == 1) {Prog += "}";}

  TokenNode *ElseNode = node->getElseNode();
  if (ElseNode) ElseNode->accept(this);
  // if (selection.count(ElseNode) == 1) {Prog += "{";}
  Stmt *else_stmt = node->getElse();
  if (else_stmt) else_stmt->accept(this);
  // if (selection.count(ElseNode) == 1) {Prog += "}";}
}
void Generator::visit(v2::SwitchStmt *node){
  TokenNode *SwitchNode = node->getSwitchNode();
  assert(SwitchNode);

  if (selection.count(SwitchNode)==1) {
    Prog += "// " + node->getASTContext()->getFileName() + ":"
      + std::to_string(node->getBeginLoc().getLine()) + "\n";
  }
  SwitchNode->accept(this);
  if (selection.count(SwitchNode) == 1) Prog += "(";
  Expr *cond = node->getCond();
  if (cond) cond->accept(this);
  if (selection.count(SwitchNode) == 1) Prog += ")";
  if (selection.count(SwitchNode) == 1) Prog += "{\n";
  std::vector<SwitchCase*> cases = node->getCases();
  for (SwitchCase *c : cases) {
    if (c) c->accept(this);
  }
  if (selection.count(SwitchNode) == 1) Prog += "}\n";
}
void Generator::visit(v2::CaseStmt *node){
  TokenNode *token = node->getCaseNode();
  if (token) token->accept(this);
  Expr *cond = node->getCond();
  if (cond) cond->accept(this);
  if (selection.count(token) == 1) {
    // HACK also add an empty statement because:
    // error: label at end of compound statement: expected statement
    Prog += ": ;";
  }
  vector<Stmt*> body = node->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void Generator::visit(v2::DefaultStmt *node){
  TokenNode *token = node->getDefaultNode();
  if (token) token->accept(this);
  if (selection.count(token) == 1) {
    // HACK also add an empty statement because:
    // error: label at end of compound statement: expected statement
    Prog += ": ;";
  }
  vector<Stmt*> body = node->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
// loop
void Generator::visit(v2::ForStmt *node){
  TokenNode *ForNode = node->getForNode();
  assert(ForNode);
  ForNode->accept(this);
  if (selection.count(ForNode) == 1) Prog += "(";
  Expr *init = node->getInit();
  if (init) init->accept(this);
  if (selection.count(ForNode) == 1) Prog += ";";
  Expr *cond = node->getCond();
  if (cond) cond->accept(this);
  if (selection.count(ForNode) == 1) Prog += ";";
  Expr *inc = node->getInc();
  if (inc) inc->accept(this);
  if (selection.count(ForNode) == 1) Prog += ")";
  Stmt *body = node->getBody();
  if (body) body->accept(this);
}
void Generator::visit(v2::WhileStmt *node){
  TokenNode *WhileNode = node->getWhileNode();
  if (WhileNode) WhileNode->accept(this);
  if (selection.count(WhileNode) == 1) Prog += "(";
  Expr *cond = node->getCond();
  if (cond) cond->accept(this);
  if (selection.count(WhileNode) == 1) Prog += ")";
  Stmt *body = node->getBody();
  if (body) body->accept(this);
}
void Generator::visit(v2::DoStmt *node){
  TokenNode *DoNode = node->getDoNode();
  TokenNode *WhileNode = node->getWhileNode();
  assert(DoNode);
  assert(WhileNode);
  DoNode->accept(this);
  Stmt *body = node->getBody();
  if (body) body->accept(this);
  WhileNode->accept(this);
  if (selection.count(WhileNode) == 1) {Prog += "(";}
  Expr *cond = node->getCond();
  if (cond) cond->accept(this);
  if (selection.count(WhileNode) == 1) {Prog += ");";}
}
// single
void Generator::visit(v2::BreakStmt *node){
  if (selection.count(node)) {
    Prog += "break;\n";
  }
}
void Generator::visit(v2::ContinueStmt *node){
  if (selection.count(node)) {
    Prog += "continue;\n";
  }
}
void Generator::visit(v2::ReturnStmt *node){
  TokenNode *ReturnNode = node->getReturnNode();
  if (ReturnNode) {
    // if adjust return, don't output the return
    if (!AdjustReturn) {
      ReturnNode->accept(this);
    }
  }
  Expr *expr = node->getValue();
  if (expr) expr->accept(this);
  if (selection.count(ReturnNode) == 1) {
    Prog += ";\n";
  }
}
// expr stmt
void Generator::visit(v2::Expr *node){
  if (selection.count(node) == 1) {
    Prog += node->getText();
  }
}
void Generator::visit(v2::DeclStmt *node){
  if (selection.count(node) == 1) {
    Prog += node->getText() + "\n";
  }
}
void Generator::visit(v2::ExprStmt *node){
  if (selection.count(node) == 1) {
    // no need semi-colon because <expr_stmt>... ;</expr_stmt>
    Prog += "// " + node->getASTContext()->getFileName() + ":"
      + std::to_string(node->getBeginLoc().getLine()) + "\n";
    Prog += node->getText() + "\n";
  }
}
