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


void LevelVisitor::visit(v2::TokenNode *token, void *data) {
  Levels[token] = lvl;
  return;
}

void LevelVisitor::visit(v2::TranslationUnitDecl *unit, void *data) {
  Levels[unit] = lvl;
  std::vector<ASTNodeBase*> decls = unit->getDecls();
  lvl++;
  for (ASTNodeBase *decl : decls) {
    // this->visit(decl);
    decl->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::FunctionDecl *function, void *data) {
  Levels[function] = lvl;
  Stmt *body = function->getBody();
  lvl++;
  // this->visit(body);
  body->accept(this);
  lvl--;
}

void LevelVisitor::visit(v2::DeclStmt *decl_stmt, void *data) {
  Levels[decl_stmt] = lvl;
}
void LevelVisitor::visit(v2::ExprStmt *expr_stmt, void *data) {
  Levels[expr_stmt] = lvl;
}
void LevelVisitor::visit(v2::CompoundStmt *comp_stmt, void *data) {
  Levels[comp_stmt] = lvl;
  std::vector<Stmt*> body = comp_stmt->getBody();
  lvl++;
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::ForStmt *for_stmt, void *data) {
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
void LevelVisitor::visit(v2::WhileStmt *while_stmt, void *data) {
  Levels[while_stmt] = lvl;
  lvl++;
  Expr *cond = while_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
  lvl--;
}
void LevelVisitor::visit(v2::DoStmt *do_stmt, void *data) {
  Levels[do_stmt] = lvl;
  lvl++;
  Expr *cond = do_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *stmt = do_stmt->getBody();
  if (stmt) stmt->accept(this);
  lvl--;
}
void LevelVisitor::visit(v2::BreakStmt *break_stmt, void *data) {
  Levels[break_stmt] = lvl;
}
void LevelVisitor::visit(v2::ContinueStmt *cont_stmt, void *data) {
  Levels[cont_stmt] = lvl;
}
void LevelVisitor::visit(v2::ReturnStmt *ret_stmt, void *data) {
  Levels[ret_stmt] = lvl;
}
void LevelVisitor::visit(v2::IfStmt *if_stmt, void *data) {
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
void LevelVisitor::visit(v2::SwitchStmt *switch_stmt, void *data) {
  Levels[switch_stmt] = lvl;
  lvl++;
  Expr *cond = switch_stmt->getCond();
  if (cond) cond->accept(this);
  switch_stmt->getCases();
  lvl--;
}

void LevelVisitor::visit(v2::CaseStmt *case_stmt, void *data) {
  Levels[case_stmt] = lvl;
  lvl++;
  std::vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    stmt->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::DefaultStmt *def_stmt, void *data) {
  Levels[def_stmt] = lvl;
  lvl++;
  std::vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    stmt->accept(this);
  }
  lvl--;
}
void LevelVisitor::visit(v2::Expr *expr, void *data) {
  Levels[expr] = lvl;
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





void TokenVisitor::visit(v2::TokenNode *token, void *data) {
  IdMap[token] = id;
  Tokens.push_back(token);
  id++;
}
void TokenVisitor::visit(v2::TranslationUnitDecl *unit, void *data) {
  std::vector<ASTNodeBase*> nodes = unit->getDecls();
  for (ASTNodeBase *node : nodes) {
    if (node) node->accept(this);
  }
}
void TokenVisitor::visit(v2::FunctionDecl *function, void *data) {
  TokenNode *ReturnNode = function->getReturnTypeNode();
  if (ReturnNode) ReturnNode->accept(this);
  TokenNode *NameNode = function->getNameNode();
  if (NameNode) NameNode->accept(this);
  TokenNode *ParamNode = function->getParamNode();
  if (ParamNode) ParamNode->accept(this);
  Stmt *body = function->getBody();
  if (body) body->accept(this);
}
void TokenVisitor::visit(v2::DeclStmt *decl_stmt, void *data) {
  IdMap[decl_stmt] = id;
  Tokens.push_back(decl_stmt);
  id++;
}
void TokenVisitor::visit(v2::ExprStmt *expr_stmt, void *data) {
  IdMap[expr_stmt] = id;
  Tokens.push_back(expr_stmt);
  id++;
}
void TokenVisitor::visit(v2::CompoundStmt *comp_stmt, void *data) {
  std::vector<Stmt*> stmts = comp_stmt->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) stmt->accept(this);
  }
}
void TokenVisitor::visit(v2::ForStmt *for_stmt, void *data) {
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
void TokenVisitor::visit(v2::WhileStmt *while_stmt, void *data) {
  TokenNode *token = while_stmt->getWhileNode();
  if (token) token->accept(this);
  Expr *cond = while_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
}
void TokenVisitor::visit(v2::DoStmt *do_stmt, void *data) {
  TokenNode *token = do_stmt->getDoNode();
  if (token) token->accept(this);
  Stmt *body = do_stmt->getBody();
  if (body) body->accept(this);
  token = do_stmt->getWhileNode();
  if (token) token->accept(this);
  Expr *cond = do_stmt->getCond();
  if (cond) cond->accept(this);
}
void TokenVisitor::visit(v2::BreakStmt *break_stmt, void *data) {
  IdMap[break_stmt] = id;
  Tokens.push_back(break_stmt);
  id++;
}
void TokenVisitor::visit(v2::ContinueStmt *cont_stmt, void *data) {
  IdMap[cont_stmt] = id;
  Tokens.push_back(cont_stmt);
  id++;
}
void TokenVisitor::visit(v2::ReturnStmt *ret_stmt, void *data) {
  IdMap[ret_stmt] = id;
  // Tokens.push_back(ret_stmt);
  TokenNode *ReturnNode = ret_stmt->getReturnNode();
  if (ReturnNode) ReturnNode->accept(this);
  Expr *value = ret_stmt->getValue();
  if (value) value->accept(this);
  id++;
}
void TokenVisitor::visit(v2::IfStmt *if_stmt, void *data) {
  TokenNode *token = if_stmt->getIfNode();
  if (token) token->accept(this);
  Expr *expr = if_stmt->getCond();
  if (expr) expr->accept(this);
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) then_stmt->accept(this);
  token = if_stmt->getElseNode();
  if (token) token->accept(this);
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) else_stmt->accept(this);
}
void TokenVisitor::visit(v2::SwitchStmt *switch_stmt, void *data) {
  TokenNode *token = switch_stmt->getSwitchNode();
  if (token) token->accept(this);
  Expr *cond = switch_stmt->getCond();
  if (cond) cond->accept(this);
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  for (SwitchCase *c : cases) {
    if (c) c->accept(this);
  }
}
void TokenVisitor::visit(v2::CaseStmt *case_stmt, void *data) {
  TokenNode *token = case_stmt->getCaseNode();
  if (token) token->accept(this);
  Expr *cond = case_stmt->getCond();
  if (cond) cond->accept(this);
  vector<Stmt*> body = case_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void TokenVisitor::visit(v2::DefaultStmt *def_stmt, void *data) {
  TokenNode *token = def_stmt->getDefaultNode();
  if (token) token->accept(this);
  vector<Stmt*> body = def_stmt->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
}
void TokenVisitor::visit(v2::Expr *expr, void *data) {
  IdMap[expr] = id;
  Tokens.push_back(expr);
  id++;
}











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





void StandAloneGrammarPatcher::process() {
  worklist = selection;
  patch = selection; // this is the result
  // get the lowest level nodes
  LevelVisitor *levelVisitor = new LevelVisitor();
  TranslationUnitDecl *unit = AST->getTranslationUnitDecl();
  unit->accept(levelVisitor);
  ASTNodeBase *node = levelVisitor->getLowestLevelNode(worklist);
  worklist.erase(node);
  ParentIndexer *parentIndexer = new ParentIndexer();
  unit->accept(parentIndexer);
  // for one of them, get all the siblings
  // remove them from worklist, and process
  // parent = getparent(sel)
  ASTNodeBase *parent = parentIndexer->getParent(node);
  set<ASTNodeBase*> children = parentIndexer->getChildren(parent);
  set<ASTNodeBase*> siblings;
  for (auto *c : children) {
    if (selection.count(c) == 1) {
      siblings.insert(c);
      worklist.erase(c);
    }
  }
  siblings.insert(node);
  // patch = matchMin(parent, sel)
  std::set<ASTNodeBase*> p = matchMin(parent, siblings);
  // ret.add(patch)
  patch.insert(p.begin(), p.end());
  // worklist.add(parent);
  worklist.insert(parent);
}



std::set<ASTNodeBase*> StandAloneGrammarPatcher::matchMin(v2::ASTNodeBase *parent, std::set<v2::ASTNodeBase*> sel) {
  // get rules
  // for each rule
  // T=body, S=children
  // seqs = subseq(T,S)
  // for seq in seqs
  // if (sel \in seq) we found the valid
  // expand = matchMin(extra, \empty)
  // return min(patch)

  // I'm actually going to implement this closely with each AST node
  // GrammarPatcher grammarPatcher(parent, sel);
  GrammarPatcher grammarPatcher;
  PatchData data;
  data.selection = sel;
  if (parent) parent->accept(&grammarPatcher, &data);
  return grammarPatcher.getPatch();
}




// TODO
void GrammarPatcher::visit(v2::TokenNode *token, void *data) {}
void GrammarPatcher::visit(v2::TranslationUnitDecl *unit, void *data) {}
void GrammarPatcher::visit(v2::FunctionDecl *function, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  // select the whole thing because I need to keep the signature of the function
  TokenNode *token = function->getReturnTypeNode();
  if (token) patch.insert(token);
  token = function->getNameNode();
  if (token) patch.insert(token);
  token = function->getParamNode();
  if (token) patch.insert(token);
  Stmt *body = function->getBody();
  // body is special.
  if (body) {
    if (selection.count(body) == 0) {
      // no selection data is passed in. Treat as no selection.
      patch.insert(body);
      body->accept(this);
    }
  }
}
void GrammarPatcher::visit(v2::DeclStmt *decl_stmt, void *data) {}
void GrammarPatcher::visit(v2::ExprStmt *expr_stmt, void *data) {}
/**
 * CompoundStmt ::= stmt*
 * No need to select anything
 */
void GrammarPatcher::visit(v2::CompoundStmt *comp_stmt, void *data) {}
void GrammarPatcher::visit(v2::ForStmt *for_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *token = for_stmt->getForNode();
  if (token) {
    if (selection.count(token) == 0) {
      patch.insert(token);
    }
  }
  // no need these
  // Expr *init = for_stmt->getInit();
  // Expr *cond = for_stmt->getCond();
  // Expr *inc = for_stmt->getInc();
  Stmt *body = for_stmt->getBody();
  if (body) {
    if (selection.count(body) == 0) {
      patch.insert(body);
      body->accept(this);
    }
  }
}
void GrammarPatcher::visit(v2::WhileStmt *while_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *token = while_stmt->getWhileNode();
  if (token) {
    // token->accept(this);
    patch.insert(token);
  }
  Expr *cond = while_stmt->getCond();
  if (cond) {
    patch.insert(cond);
    // cond->accept(this);
  }
  Stmt *body = while_stmt->getBody();
  if (body) {
    if (selection.count(body) == 0) {
      patch.insert(body);
      body->accept(this);
    }
  }
}
void GrammarPatcher::visit(v2::DoStmt *do_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *token = do_stmt->getDoNode();
  if (token) {
    // token->accept(this);
    patch.insert(token);
  }
  Stmt *body = do_stmt->getBody();
  if (body) {
    if (selection.count(body) == 0) {
      patch.insert(body);
      body->accept(this);
    }
  }
  token = do_stmt->getWhileNode();
  if (token) {
    patch.insert(token);
    // token->accept(this);
  }
  Expr *cond = do_stmt->getCond();
  if (cond) {
    // cond->accept(this);
    patch.insert(cond);
  }
}
void GrammarPatcher::visit(v2::BreakStmt *break_stmt, void *data) {}
void GrammarPatcher::visit(v2::ContinueStmt *cont_stmt, void *data) {}
void GrammarPatcher::visit(v2::ReturnStmt *ret_stmt, void *data) {
  TokenNode *ReturnNode = ret_stmt->getReturnNode();
  if (ReturnNode) {
    // ReturnNode->accept(this);
    patch.insert(ReturnNode);
  }
  // add value because it is related to function signature
  Expr *value = ret_stmt->getValue();
  if (value) {
    // value->accept(this);
    patch.insert(value);
  }
}
void GrammarPatcher::visit(v2::IfStmt *if_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *token = if_stmt->getIfNode();
  if (token) {
    // token->accept(this);
    patch.insert(token);
  }
  Expr *expr = if_stmt->getCond();
  if (expr) {
    patch.insert(expr);
    // expr->accept(this);
  }
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) {
    if (selection.count(then_stmt) == 0) {
      patch.insert(then_stmt);
      then_stmt->accept(this);
    }
  }
  TokenNode *ElseNode = if_stmt->getElseNode();
  Stmt *else_stmt = if_stmt->getElse();
  if (selection.count(ElseNode) == 1 || selection.count(else_stmt) == 1) {
    if (ElseNode) {
      patch.insert(ElseNode);
    }
    if (else_stmt) {
      patch.insert(else_stmt);
      else_stmt->accept(this);
    }
  }
}
void GrammarPatcher::visit(v2::SwitchStmt *switch_stmt, void *data) {
  TokenNode *token = switch_stmt->getSwitchNode();
  if (token) {
    patch.insert(token);
    // token->accept(this);
  }
  Expr *cond = switch_stmt->getCond();
  if (cond) {
    // cond->accept(this);
    patch.insert(cond);
  }
  // no need for cases
  // std::vector<SwitchCase*> cases = switch_stmt->getCases();
  // for (SwitchCase *c : cases) {
  //   if (c) c->accept(this);
  // }
}
void GrammarPatcher::visit(v2::CaseStmt *case_stmt, void *data) {
  TokenNode *token = case_stmt->getCaseNode();
  if (token) {
    patch.insert(token);
  }
  Expr *cond = case_stmt->getCond();
  if (cond) {
    patch.insert(cond);
  }
  // no need body
  // vector<Stmt*> body = case_stmt->getBody();
  // for (Stmt *stmt : body) {
  //   if (stmt) stmt->accept(this);
  // }
}
void GrammarPatcher::visit(v2::DefaultStmt *def_stmt, void *data) {
  TokenNode *token = def_stmt->getDefaultNode();
  if (token) {
    patch.insert(token);
  }
}
void GrammarPatcher::visit(v2::Expr *expr, void *data) {}









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









// TODO
void Generator::visit(v2::TokenNode *token, void *data) {
  if (selection.count(token) == 1) {
    Prog += token->getText();
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
    Prog += decl_stmt->getText() + ";\n";
  }
}
void Generator::visit(v2::ExprStmt *expr_stmt, void *data) {
  if (selection.count(expr_stmt) == 1) {
    Prog += expr_stmt->getText() + ";\n";
  }
}
void Generator::visit(v2::CompoundStmt *comp_stmt, void *data) {
  // FIXME ParenNode
  Prog += "{\n";
  std::vector<Stmt*> stmts = comp_stmt->getBody();
  for (Stmt *stmt : stmts) {
    if (stmt) stmt->accept(this);
  }
  Prog += "}\n";
}
void Generator::visit(v2::ForStmt *for_stmt, void *data) {
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
void Generator::visit(v2::WhileStmt *while_stmt, void *data) {
  TokenNode *token = while_stmt->getWhileNode();
  if (token) token->accept(this);
  Expr *cond = while_stmt->getCond();
  if (cond) cond->accept(this);
  Stmt *body = while_stmt->getBody();
  if (body) body->accept(this);
}
void Generator::visit(v2::DoStmt *do_stmt, void *data) {
  TokenNode *token = do_stmt->getDoNode();
  if (token) token->accept(this);
  Stmt *body = do_stmt->getBody();
  if (body) body->accept(this);
  token = do_stmt->getWhileNode();
  if (token) token->accept(this);
  Expr *cond = do_stmt->getCond();
  if (cond) cond->accept(this);
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
  if (ReturnNode) ReturnNode->accept(this);
  Expr *expr = ret_stmt->getValue();
  if (expr) expr->accept(this);
  Prog += ";\n";
}
void Generator::visit(v2::IfStmt *if_stmt, void *data) {
  TokenNode *IfNode = if_stmt->getIfNode();
  if (IfNode) IfNode->accept(this);
  Expr *expr = if_stmt->getCond();
  if (expr) expr->accept(this);
  if (selection.count(IfNode) == 1) {
    Prog += "{";
  }
  Stmt *then_stmt = if_stmt->getThen();
  if (then_stmt) then_stmt->accept(this);
  if (selection.count(IfNode) == 1) {
    Prog += "}";
  }
  TokenNode *ElseNode = if_stmt->getElseNode();
  if (ElseNode) ElseNode->accept(this);
  if (selection.count(ElseNode) == 1) {
    Prog += "{";
  }
  Stmt *else_stmt = if_stmt->getElse();
  if (else_stmt) else_stmt->accept(this);
  if (selection.count(ElseNode) == 1) {
    Prog += "}";
  }
}
void Generator::visit(v2::SwitchStmt *switch_stmt, void *data) {
  TokenNode *SwitchNode = switch_stmt->getSwitchNode();
  if (SwitchNode) SwitchNode->accept(this);
  Expr *cond = switch_stmt->getCond();
  if (cond) cond->accept(this);
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  for (SwitchCase *c : cases) {
    if (c) c->accept(this);
  }
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
