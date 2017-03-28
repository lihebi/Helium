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

void Printer::visit(TokenNode *token, void *data) {
  assert(token);
  os << "(token " + token->getText() << ")";
}

void Printer::visit(TranslationUnitDecl *unit, void *data) {
  os << "(unit ";
  std::vector<ASTNodeBase*> nodes = unit->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) node->accept(this);
  }
  os << ")\n";
}
void Printer::visit(FunctionDecl *function, void *datan){
  os << "(function:" << function->getName() << "\n";
  Stmt *body = function->getBody();
  body->accept(this);
  os << ")\n";
}
void Printer::visit(DeclStmt *decl_stmt, void *datat){
  os << "(decl_stmt)";
}
void Printer::visit(ExprStmt *expr_stmt, void *datat){
  os << "(expr_stmt)";
}
void Printer::visit(CompoundStmt *comp_stmt, void *datat){
  os << "(comp_stmt \n";
  std::vector<Stmt*> stmts = comp_stmt->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) stmt->accept(this);
  }
  os << ")\n";
}
void Printer::visit(ForStmt *for_stmt, void *datat){
  os << "(for_stmt \n";
  Stmt *body = for_stmt->getBody();
  if (body) body->accept(this);
  os << ")\n";
}
void Printer::visit(WhileStmt *while_stmt, void *datat){
  os << "(while_stmt \n";
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
  os << ")\n";
}
void Printer::visit(DoStmt *do_stmt, void *datat){
  os << "(do_stmt \n";
  Stmt *body = do_stmt->getBody();
  if (body) body->accept(this);
  os << ")\n";
}
void Printer::visit(BreakStmt *break_stmt, void *datat){
  os << "(break_stmt)";
}
void Printer::visit(ContinueStmt *cont_stmt, void *datat){
  os << "(cont_stmt)";
}
void Printer::visit(ReturnStmt *ret_stmt, void *datat){
  os << "(ret_stmt)";
}
void Printer::visit(IfStmt *if_stmt, void *datat){
  os << "(if_stmt \n";
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) then_stmt->accept(this);
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) else_stmt->accept(this);
  os << ")\n";
}
void Printer::visit(SwitchStmt *switch_stmt, void *datat){
  os << "(switch_stmt \n";
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  for (SwitchCase *c : cases) {
    c->accept(this);
  }
  os << ")\n";
}
void Printer::visit(CaseStmt *case_stmt, void *datat){
  os << "(case_stmt ";
  vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
  os << ")\n";
}
void Printer::visit(DefaultStmt *def_stmt, void *datat){
  os << "(def_stmt ";
  vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
  os << ")";
}
void Printer::visit(Expr *exp, void *datar){
  os << "(expr_stmt)";
}


