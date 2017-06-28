#include "helium/parser/Visitor.h"

// high level
void InstrumentPointVisitor::visit(TokenNode *node) {}
void InstrumentPointVisitor::visit(TranslationUnitDecl *node) {}
void InstrumentPointVisitor::visit(FunctionDecl *node) {
  CompoundStmt *compstmt =
    dynamic_cast<CompoundStmt*>(node->getBody());
  std::vector<Stmt*> stmts = compstmt->getBody();
  if (!stmts.empty()) {
    Stmt *first = *stmts.begin();
    Stack.push_back({true, first});
    Visitor::visit(node);
    Stack.pop_back();
  } else {
    Visitor::visit(node);
  }
}
void InstrumentPointVisitor::visit(CompoundStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
// condition
void InstrumentPointVisitor::visit(IfStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(SwitchStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(CaseStmt *node) {
}
void InstrumentPointVisitor::visit(DefaultStmt *node) {
}
// loop
void InstrumentPointVisitor::visit(ForStmt *node) {
  stmt(node);
  Visitor::visit(node);
}

/**
 * Common statement
 */
void InstrumentPointVisitor::stmt(Stmt *node) {
  After[node] = {false, node};
}
void InstrumentPointVisitor::visit(WhileStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(DoStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
// single
void InstrumentPointVisitor::visit(BreakStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(ContinueStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(ReturnStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
// expr stmt
void InstrumentPointVisitor::visit(Expr *node) {
  // rely on the top of stack
  After[node] = Stack.top();
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(DeclStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(ExprStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
