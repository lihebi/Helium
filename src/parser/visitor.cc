#include "helium/parser/visitor.h"
#include "helium/parser/ast_v2.h"
#include "helium/parser/source_manager.h"
#include "helium/utils/string_utils.h"
#include <iostream>

using std::vector;
using std::string;

using namespace v2;


string Printer::PrettyPrint(string ast) {
  // join line if ) is on a single line
  vector<string> lines = utils::split(ast, '\n');
  vector<string> ret;
  string tmp;
  for (string line : lines) {
    utils::trim(line);
    if (line.size() == 1 && line[0] == ')') {
      tmp += ')';
    } else if (line.empty()) {
      continue;
    } else {
      ret.push_back(tmp);
      tmp = line;
    }
  }
  ret.push_back(tmp);
  string retstr;
  // indent
  int indent = 0;
  for (string line : ret) {
    int open = std::count(line.begin(), line.end(), '(');
    int close = std::count(line.begin(), line.end(), ')');
    retstr += string(indent*2, ' ') + line + "\n";
    indent = indent + open - close;
  }
  return retstr;
}


void LevelVisitor::visit(v2::TokenNode *token) {
  Levels[token] = lvl;
  return;
}

void LevelVisitor::visit(v2::TranslationUnitDecl *unit) {
  Levels[unit] = lvl;
  std::vector<ASTNodeBase*> decls = unit->getDecls();
  lvl++;
  for (ASTNodeBase *decl : decls) {
    // this->visit(decl);
    decl->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::FunctionDecl *function) {
  Levels[function] = lvl;
  Stmt *body = function->getBody();
  lvl++;
  // this->visit(body);
  body->accept(this);
  lvl--;
}

void LevelVisitor::visit(v2::DeclStmt *decl_stmt) {
  Levels[decl_stmt] = lvl;
}
void LevelVisitor::visit(v2::ExprStmt *expr_stmt) {
  Levels[expr_stmt] = lvl;
}
void LevelVisitor::visit(v2::CompoundStmt *comp_stmt) {
  Levels[comp_stmt] = lvl;
  std::vector<Stmt*> body = comp_stmt->getBody();
  lvl++;
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::ForStmt *for_stmt) {
  Levels[for_stmt] = lvl;
  lvl++;
  Expr *init = for_stmt->getInit();
  if (init) init->accept(this);
  Expr *cond = for_stmt->getCond();
  if (cond) cond->accept(this);
  Expr *inc = for_stmt->getInc();
  if (inc) inc->accept(this);
  Stmt *body = for_stmt->getBody();
  if (body) body->accept(this);
  lvl--;
}
void LevelVisitor::visit(v2::WhileStmt *while_stmt) {
  Levels[while_stmt] = lvl;
  lvl++;
  Expr *cond = while_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
  lvl--;
}
void LevelVisitor::visit(v2::DoStmt *do_stmt) {
  Levels[do_stmt] = lvl;
  lvl++;
  Expr *cond = do_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *stmt = do_stmt->getBody();
  if (stmt) stmt->accept(this);
  lvl--;
}
void LevelVisitor::visit(v2::BreakStmt *break_stmt) {
  Levels[break_stmt] = lvl;
}
void LevelVisitor::visit(v2::ContinueStmt *cont_stmt) {
  Levels[cont_stmt] = lvl;
}
void LevelVisitor::visit(v2::ReturnStmt *ret_stmt) {
  Levels[ret_stmt] = lvl;
}
void LevelVisitor::visit(v2::IfStmt *if_stmt) {
  Levels[if_stmt] = lvl;
  lvl++;
  Expr *cond = if_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) then_stmt->accept(this);
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) else_stmt->accept(this);
  lvl--;
}
void LevelVisitor::visit(v2::SwitchStmt *switch_stmt) {
  Levels[switch_stmt] = lvl;
  lvl++;
  Expr *cond = switch_stmt->getCond();
  if (cond) cond->accept(this);
  switch_stmt->getCases();
  lvl--;
}

void LevelVisitor::visit(v2::CaseStmt *case_stmt) {
  Levels[case_stmt] = lvl;
  lvl++;
  std::vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    stmt->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::DefaultStmt *def_stmt) {
  Levels[def_stmt] = lvl;
  lvl++;
  std::vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    stmt->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::Expr *expr) {
  Levels[expr] = lvl;
}











void Printer::visit(TokenNode *token) {
  assert(token);
  os << "(token " + token->getText() << ")";
}

void Printer::visit(TranslationUnitDecl *unit) {
  os << "(unit ";
  std::vector<ASTNodeBase*> nodes = unit->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) node->accept(this);
  }
  os << ")\n";
}
void Printer::visit(FunctionDecl *function){
  os << "(function:" << function->getName() << "\n";
  Stmt *body = function->getBody();
  body->accept(this);
  os << ")\n";
}
void Printer::visit(DeclStmt *decl_stmt){
  os << "(decl_stmt)";
}
void Printer::visit(ExprStmt *expr_stmt){
  os << "(expr_stmt)";
}
void Printer::visit(CompoundStmt *comp_stmt){
  os << "(comp_stmt \n";
  std::vector<Stmt*> stmts = comp_stmt->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) stmt->accept(this);
  }
  os << ")\n";
}
void Printer::visit(ForStmt *for_stmt){
  os << "(for_stmt \n";
  Stmt *body = for_stmt->getBody();
  if (body) body->accept(this);
  os << ")\n";
}
void Printer::visit(WhileStmt *while_stmt){
  os << "(while_stmt \n";
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
  os << ")\n";
}
void Printer::visit(DoStmt *do_stmt){
  os << "(do_stmt \n";
  Stmt *body = do_stmt->getBody();
  if (body) body->accept(this);
  os << ")\n";
}
void Printer::visit(BreakStmt *break_stmt){
  os << "(break_stmt)";
}
void Printer::visit(ContinueStmt *cont_stmt){
  os << "(cont_stmt)";
}
void Printer::visit(ReturnStmt *ret_stmt){
  os << "(ret_stmt)";
}
void Printer::visit(IfStmt *if_stmt){
  os << "(if_stmt \n";
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) then_stmt->accept(this);
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) else_stmt->accept(this);
  os << ")\n";
}
void Printer::visit(SwitchStmt *switch_stmt){
  os << "(switch_stmt \n";
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  for (SwitchCase *c : cases) {
    c->accept(this);
  }
  os << ")\n";
}
void Printer::visit(CaseStmt *case_stmt){
  os << "(case_stmt ";
  vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
  os << ")\n";
}
void Printer::visit(DefaultStmt *def_stmt){
  os << "(def_stmt ";
  vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
  os << ")";
}
void Printer::visit(Expr *expr){
  os << "(expr_stmt)";
}





void TokenVisitor::visit(v2::TokenNode *token) {
  IdMap[token] = id;
  id++;
}
void TokenVisitor::visit(v2::TranslationUnitDecl *unit) {
  std::vector<ASTNodeBase*> nodes = unit->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) node->accept(this);
  }
}
void TokenVisitor::visit(v2::FunctionDecl *function) {
  TokenNode *token = function->getReturnTypeNode();
  if (token) token->accept(this);
  token = function->getNameNode();
  if (token) token->accept(this);
  token = function->getParamNode();
  if (token) token->accept(this);
  Stmt *body = function->getBody();
  body->accept(this);
}
void TokenVisitor::visit(v2::DeclStmt *decl_stmt) {
  IdMap[decl_stmt] = id;
  id++;
}
void TokenVisitor::visit(v2::ExprStmt *expr_stmt) {
  IdMap[expr_stmt] = id;
  id++;
}
void TokenVisitor::visit(v2::CompoundStmt *comp_stmt) {
  std::vector<Stmt*> stmts = comp_stmt->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) stmt->accept(this);
  }
}
void TokenVisitor::visit(v2::ForStmt *for_stmt) {
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
void TokenVisitor::visit(v2::WhileStmt *while_stmt) {
  TokenNode *token = while_stmt->getWhileNode();
  if (token) token->accept(this);
  Expr *cond = while_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
}
void TokenVisitor::visit(v2::DoStmt *do_stmt) {
  TokenNode *token = do_stmt->getDoNode();
  if (token) token->accept(this);
  Stmt *body = do_stmt->getBody();
  if (body) body->accept(this);
  token = do_stmt->getWhileNode();
  if (token) token->accept(this);
  Expr *cond = do_stmt->getCond();
  if (cond) cond->accept(this);
}
void TokenVisitor::visit(v2::BreakStmt *break_stmt) {
  IdMap[break_stmt] = id;
  id++;
}
void TokenVisitor::visit(v2::ContinueStmt *cont_stmt) {
  IdMap[cont_stmt] = id;
  id++;
}
void TokenVisitor::visit(v2::ReturnStmt *ret_stmt) {
  IdMap[ret_stmt] = id;
  id++;
}
void TokenVisitor::visit(v2::IfStmt *if_stmt) {
  TokenNode *token = if_stmt->getIfNode();
  if (token) token->accept(this);
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) then_stmt->accept(this);
  token = if_stmt->getElseNode();
  if (token) token->accept(this);
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) else_stmt->accept(this);
}
void TokenVisitor::visit(v2::SwitchStmt *switch_stmt) {
  TokenNode *token = switch_stmt->getSwitchNode();
  if (token) token->accept(this);
  Expr *cond = switch_stmt->getCond();
  if (cond) cond->accept(this);
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  for (SwitchCase *c : cases) {
    c->accept(this);
  }
}
void TokenVisitor::visit(v2::CaseStmt *case_stmt) {
  TokenNode *token = case_stmt->getCaseNode();
  if (token) token->accept(this);
  Expr *cond = case_stmt->getCond();
  if (cond) cond->accept(this);
  vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void TokenVisitor::visit(v2::DefaultStmt *def_stmt) {
  TokenNode *token = def_stmt->getDefaultNode();
  if (token) token->accept(this);
  vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void TokenVisitor::visit(v2::Expr *expr) {
  IdMap[expr] = id;
  id++;
}
