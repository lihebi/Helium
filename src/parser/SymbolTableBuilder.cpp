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
void SymbolTableBuilder::visit(v2::TokenNode *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(v2::TranslationUnitDecl *node) {
  Table.pushScope();
  insertDefUse(node);
  Visitor::visit(node);
  Table.popScope();
}
void SymbolTableBuilder::visit(v2::FunctionDecl *node) {
  Table.pushScope();
  std::set<std::string> vars = node->getVars();
  Table.add(vars, node);
  insertDefUse(node);
  Visitor::visit(node);
  Table.popScope();
}
void SymbolTableBuilder::visit(v2::CompoundStmt *node) {
  Table.pushScope();
  insertDefUse(node);
  Visitor::visit(node);
  Table.popScope();
}
// condition
void SymbolTableBuilder::visit(v2::IfStmt *node) {
  // it is the then and else COMPOUND Statement that creates scope
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(v2::SwitchStmt *node) {
  Table.pushScope();
  insertDefUse(node);
  Visitor::visit(node);
  Table.popScope();
}
void SymbolTableBuilder::visit(v2::CaseStmt *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(v2::DefaultStmt *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
// loop
void SymbolTableBuilder::visit(v2::ForStmt *node) {
  Table.pushScope();
  std::set<std::string> vars = node->getVars();
  Table.add(vars, node);
  insertDefUse(node);
  Visitor::visit(node);
  Table.popScope();
}
void SymbolTableBuilder::visit(v2::WhileStmt *node) {
  Table.pushScope();
  insertDefUse(node);
  Visitor::visit(node);
  Table.popScope();
}
void SymbolTableBuilder::visit(v2::DoStmt *node) {
  Table.pushScope();
  insertDefUse(node);
  Visitor::visit(node);
  Table.popScope();
}
// single
void SymbolTableBuilder::visit(v2::BreakStmt *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(v2::ContinueStmt *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(v2::ReturnStmt *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
// expr stmt
void SymbolTableBuilder::visit(v2::Expr *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(v2::DeclStmt *node) {
  std::set<std::string> vars = node->getVars();
  Table.add(vars, node);
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(v2::ExprStmt *node) {
  insertDefUse(node);
  Visitor::visit(node);
}



void SymbolTableBuilder::insertDefUse(v2::ASTNodeBase *use) {
  // for the use of variables, getvarid, and query symbol table
  // get_var_ids() requires a XMLNode, not good at all
  std::set<std::string> used_vars = use->getUsedVars();
  for (std::string var : used_vars) {
    v2::ASTNodeBase *def = Table.get(var);
    if (def) {
      Use2DefMap[use].insert(def);
    }
  }
}
