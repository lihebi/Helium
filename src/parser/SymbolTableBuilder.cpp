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
  // (HEBI: FunctionDecl)
  std::set<std::string> vars = node->getVars();
  // Table.add(vars, node);
  // map to param node
  Table.add(vars, node->getParamNode());
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
  // std::set<std::string> vars = node->getVars();
  Expr *init = node->getInit();
  std::set<std::string> vars = init->getVars();
  // Table.add(vars, node);
  // (HEBI: ForStmt -> init)
  Table.add(vars, init);
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
  // (HEBI: DeclStmt)
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
  // record the table right at the end of inserting for this node
  PersistTables[use]=Table;
}



/**
 * Dumping use2def map
 */
void SymbolTableBuilder::dump(std::ostream &os) {
  // std::map<v2::ASTNodeBase*,std::set<v2::ASTNodeBase*> > Use2DefMap;
  os << "[SymbolTableBuilder] Use2DefMap:\n";
  for (auto &m : Use2DefMap) {
    ASTNodeBase *use = m.first;
    os << "Use: ";
    use->dump(os);
    os << " Def: ";
    for (ASTNodeBase *def : m.second) {
      // getting
      def->dump(os);
      os << " ";
    }
    os << "\n";
  }
}
