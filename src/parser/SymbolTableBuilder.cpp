#include "helium/parser/Visitor.h"
#include "helium/parser/AST.h"
#include "helium/parser/SourceManager.h"
#include "helium/utils/StringUtils.h"
#include <iostream>

using std::vector;
using std::string;
using std::map;
using std::set;



// high level
void SymbolTableBuilder::visit(TokenNode *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(TranslationUnitDecl *node) {
  Table.pushScope();
  insertDefUse(node);
  Visitor::visit(node);
  Table.popScope();
}
void SymbolTableBuilder::visit(FunctionDecl *node) {
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
void SymbolTableBuilder::visit(CompoundStmt *node) {
  Table.pushScope();
  insertDefUse(node);
  Visitor::visit(node);
  Table.popScope();
}
// condition
void SymbolTableBuilder::visit(IfStmt *node) {
  // it is the then and else COMPOUND Statement that creates scope
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(SwitchStmt *node) {
  Table.pushScope();
  insertDefUse(node);
  Visitor::visit(node);
  Table.popScope();
}
void SymbolTableBuilder::visit(CaseStmt *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(DefaultStmt *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
// loop
void SymbolTableBuilder::visit(ForStmt *node) {
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
void SymbolTableBuilder::visit(WhileStmt *node) {
  Table.pushScope();
  insertDefUse(node);
  Visitor::visit(node);
  Table.popScope();
}
void SymbolTableBuilder::visit(DoStmt *node) {
  Table.pushScope();
  insertDefUse(node);
  Visitor::visit(node);
  Table.popScope();
}
// single
void SymbolTableBuilder::visit(BreakStmt *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(ContinueStmt *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(ReturnStmt *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
// expr stmt
void SymbolTableBuilder::visit(Expr *node) {
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(DeclStmt *node) {
  std::set<std::string> vars = node->getVars();
  // (HEBI: DeclStmt)
  Table.add(vars, node);
  insertDefUse(node);
  Visitor::visit(node);
}
void SymbolTableBuilder::visit(ExprStmt *node) {
  insertDefUse(node);
  Visitor::visit(node);
}



void SymbolTableBuilder::insertDefUse(ASTNodeBase *use) {
  // for the use of variables, getvarid, and query symbol table
  // get_var_ids() requires a XMLNode, not good at all
  std::set<std::string> used_vars = use->getUsedVars();
  for (std::string var : used_vars) {
    ASTNodeBase *def = Table.get(var);
    // filter out def==use
    if (def && def != use) {
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
  // std::map<ASTNodeBase*,std::set<ASTNodeBase*> > Use2DefMap;
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
