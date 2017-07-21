#include "helium/parser/Visitor.h"
#include "helium/parser/AST.h"
#include "helium/parser/SourceManager.h"
#include "helium/utils/StringUtils.h"

#include "helium/type/Type.h"
#include <iostream>

using std::vector;
using std::string;
using std::map;
using std::set;



// high level
void NewGenerator::visit(TokenNode *node){
  Visitor::visit(node);
  if (m_sel.count(node) == 1) {
    addInnerProg(node, node->getText());
  }
}
void NewGenerator::visit(TranslationUnitDecl *node){
  Visitor::visit(node);
  std::vector<ASTNodeBase*> nodes = node->getDecls();
  std::string prog;
  for (ASTNodeBase *n : nodes) {
    std::string sub_prog = getInnerProg(n);
    prog += sub_prog;
  }
  addInnerProg(node, prog);
}
void NewGenerator::visit(FunctionDecl *node){
  Visitor::visit(node);
  TokenNode *ReturnNode = node->getReturnTypeNode();
  TokenNode *NameNode = node->getNameNode();
  TokenNode *ParamNode = node->getParamNode();
  Stmt *body = node->getBody();
  std::string ret_prog = getInnerProg(ReturnNode);
  std::string name_prog = getInnerProg(NameNode);
  std::string param_prog = getInnerProg(ParamNode);
  std::string body_prog = getInnerProg(body);
  
  std::string prog;
  if (!ret_prog.empty() && !name_prog.empty() && !param_prog.empty()) {
    // function header is selected
    // record the function name. This function should be called in helium_entry
    prog += ret_prog + " " + name_prog + "(" + param_prog + ")";
    prog += body_prog;
    // create a helium_entry_<func>() function
    std::string entry_func_name = "helium_entry_" + NameNode->getText();
    prog += "void " + entry_func_name + "() {\n";
    // TODO call the function with parameter instantialization
    prog += "  // TODO call function " + NameNode->getText() + " with properly initialized arguments\n";
    prog += "}\n";
    // record the function
    m_entry_funcs.insert(entry_func_name);
  } else if (!body_prog.empty()) {
    // function header is not selected
    // create a helium_entry_dummy_<func>() functions
    std::string entry_func_name = "helium_entry_dummy_" + NameNode->getText();
    prog += "void " + entry_func_name + "() {\n";
    prog += body_prog;
    prog += "}\n";
    m_entry_funcs.insert(entry_func_name);
  }
  addInnerProg(node, prog);
}
void NewGenerator::visit(CompoundStmt *node){
  Visitor::visit(node);
  TokenNode *lbrace = node->getLBrace();
  TokenNode *rbrace = node->getRBrace();
  std::string prog;
  std::string lbrace_prog = getInnerProg(lbrace);
  if (!lbrace_prog.empty()) {
    lbrace_prog += "\n";
  }
  std::string rbrace_prog = getInnerProg(rbrace);
  if (!rbrace_prog.empty()) {
    rbrace_prog += "\n";
  }
  prog += lbrace_prog;
  for (Stmt *stmt : node->getBody()) {
    std::string sub_prog = getInnerProg(stmt);
    prog += sub_prog;
  }
  prog += rbrace_prog;
  addInnerProg(node, prog);
}
// condition
void NewGenerator::visit(IfStmt *node){
  Visitor::visit(node);
  TokenNode *IfNode = node->getIfNode();
  Expr *cond = node->getCond();
  Stmt *then_stmt = node->getThen();
  TokenNode *ElseNode = node->getElseNode();
  Stmt *else_stmt = node->getElse();

  std::string if_node_prog = getInnerProg(IfNode);
  std::string cond_prog = getInnerProg(cond);
  std::string then_prog = getInnerProg(then_stmt);
  std::string else_node_prog = getInnerProg(ElseNode);
  std::string else_prog = getInnerProg(else_stmt);

  std::string prog;
  if (!if_node_prog.empty()) {
    // I'm adding these parenthesis back because
    // if () char a; will not compile
    prog += if_node_prog + "(" + cond_prog + ")" + then_prog + else_node_prog + else_prog;
  }
  addInnerProg(node, prog);
}
void NewGenerator::visit(SwitchStmt *node){
  Visitor::visit(node);
  TokenNode *SwitchNode = node->getSwitchNode();
  Expr *cond = node->getCond();
  std::vector<SwitchCase*> cases = node->getCases();
  std::string switch_node_prog = getInnerProg(SwitchNode);
  std::string cond_prog = getInnerProg(cond);
  std::string body_prog;
  for (SwitchCase *c : cases) {
    std::string case_prog = getInnerProg(c);
    body_prog += case_prog;
  }
  std::string prog;
  if (!switch_node_prog.empty()) {
    prog += switch_node_prog + "(" + cond_prog + ") {" + body_prog + "}\n";
  } else {
    prog += body_prog;
  }
  addInnerProg(node, prog);
}
void NewGenerator::visit(CaseStmt *node){
  Visitor::visit(node);
  TokenNode *case_node = node->getCaseNode();
  Expr *cond = node->getCond();
  vector<Stmt*> body = node->getBody();
  std::string case_node_prog = getInnerProg(case_node);
  std::string cond_prog = getInnerProg(cond);
  std::string body_prog;
  for (Stmt *stmt : body) {
    body_prog += getInnerProg(stmt);
  }
  std::string prog;
  if (!case_node_prog.empty()) {
    // HACK also add an empty statement because:
    // error: label at end of compound statement: expected statement
    // Prog += ": ;";
    prog += case_node_prog + " " + cond_prog + ":" + body_prog + ";";
  } else {
    prog += body_prog;
  }
  addInnerProg(node, prog);
}
void NewGenerator::visit(DefaultStmt *node){
  Visitor::visit(node);
  TokenNode *def_node = node->getDefaultNode();
  vector<Stmt*> body = node->getBody();
  std::string def_node_prog = getInnerProg(def_node);
  std::string body_prog;
  for (Stmt *stmt : body) {
    body_prog += getInnerProg(stmt);
  }
  std::string prog;
  if (!def_node_prog.empty()) {
    prog += def_node_prog + ": " + body_prog + ";";
  } else {
    prog += body_prog;
  }
  addInnerProg(node, prog);
}
// loop
void NewGenerator::visit(ForStmt *node){
  Visitor::visit(node);
  TokenNode *ForNode = node->getForNode();
  Expr *init = node->getInit();
  Expr *cond = node->getCond();
  Expr *inc = node->getInc();
  Stmt *body = node->getBody();

  std::string for_node_prog = getInnerProg(ForNode);
  std::string init_prog = getInnerProg(init);
  std::string cond_prog = getInnerProg(cond);
  std::string inc_prog = getInnerProg(inc);
  std::string body_prog = getInnerProg(body);

  std::string prog;
  if (!for_node_prog.empty()) {
    prog += for_node_prog + "(" + init_prog + ";" + cond_prog + ";" + inc_prog + ")" + "{" + body_prog + "}";
  } else {
    prog += body_prog;
  }
  addInnerProg(node, prog);
}
void NewGenerator::visit(WhileStmt *node){
  Visitor::visit(node);
  TokenNode *WhileNode = node->getWhileNode();
  Expr *cond = node->getCond();
  Stmt *body = node->getBody();

  std::string while_node_prog = getInnerProg(WhileNode);
  std::string cond_prog = getInnerProg(cond);
  std::string body_prog = getInnerProg(body);

  std::string prog;
  if (!while_node_prog.empty()) {
    prog += while_node_prog + "(" + cond_prog + ")" + "{" + body_prog + "}";
  } else {
    prog += body_prog;
  }
  addInnerProg(node, prog);
}
void NewGenerator::visit(DoStmt *node){
  Visitor::visit(node);
  TokenNode *DoNode = node->getDoNode();
  TokenNode *WhileNode = node->getWhileNode();
  Stmt *body = node->getBody();
  Expr *cond = node->getCond();

  std::string do_node_prog = getInnerProg(DoNode);
  std::string while_node_prog = getInnerProg(WhileNode);
  std::string body_prog = getInnerProg(body);
  std::string cond_prog = getInnerProg(cond);

  std::string prog;
  if (!do_node_prog.empty()) {
    prog += do_node_prog + body_prog + while_node_prog + "(" + cond_prog + ")" + ";";
  } else {
    prog += body_prog;
  }
  addInnerProg(node, prog);
}
// single
void NewGenerator::visit(BreakStmt *node){
  Visitor::visit(node);
  if (m_sel.count(node) == 1) {
    addInnerProg(node, "break;\n");
  }
}
void NewGenerator::visit(ContinueStmt *node){
  Visitor::visit(node);
  if (m_sel.count(node) == 1) {
    addInnerProg(node, "continue;\n");
  }
}
void NewGenerator::visit(ReturnStmt *node){
  Visitor::visit(node);
  TokenNode *ReturnNode = node->getReturnNode();
  Expr *expr = node->getValue();

  std::string ret_node_prog = getInnerProg(ReturnNode);
  std::string ret_value_prog = getInnerProg(expr);

  std::string prog;
  if (!ret_node_prog.empty()) {
    prog += ret_node_prog + " " + ret_value_prog + "\n";
  }
  // FIXME adjust return
  // FIXME select only value
  addInnerProg(node, prog);
}
// expr stmt
void NewGenerator::visit(Expr *node){
  Visitor::visit(node);
  if (m_sel.count(node) == 1) {
    addInnerProg(node, node->getText());
  }
}
void NewGenerator::visit(DeclStmt *node){
  Visitor::visit(node);
  if (m_sel.count(node) == 1) {
    // TODO add input code
    addInnerProg(node, node->getText());
  }
}
void NewGenerator::visit(ExprStmt *node){
  Visitor::visit(node);
  if (m_sel.count(node) == 1) {
    addInnerProg(node, node->getText() + "\n");
  }
}
