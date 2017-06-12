#include "helium/parser/visitor.h"

// high level
void InstrumentPointVisitor::visit(v2::TokenNode *node) {}
void InstrumentPointVisitor::visit(v2::TranslationUnitDecl *node) {}
void InstrumentPointVisitor::visit(v2::FunctionDecl *node) {
  v2::CompoundStmt *compstmt =
    dynamic_cast<v2::CompoundStmt*>(node->getBody());
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
void InstrumentPointVisitor::visit(v2::CompoundStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
// condition
void InstrumentPointVisitor::visit(v2::IfStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(v2::SwitchStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(v2::CaseStmt *node) {
}
void InstrumentPointVisitor::visit(v2::DefaultStmt *node) {
}
// loop
void InstrumentPointVisitor::visit(v2::ForStmt *node) {
  stmt(node);
  Visitor::visit(node);
}

/**
 * Common statement
 */
void InstrumentPointVisitor::stmt(v2::Stmt *node) {
  After[node] = {false, node};
}
void InstrumentPointVisitor::visit(v2::WhileStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(v2::DoStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
// single
void InstrumentPointVisitor::visit(v2::BreakStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(v2::ContinueStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(v2::ReturnStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
// expr stmt
void InstrumentPointVisitor::visit(v2::Expr *node) {
  // rely on the top of stack
  After[node] = Stack.top();
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(v2::DeclStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
void InstrumentPointVisitor::visit(v2::ExprStmt *node) {
  stmt(node);
  Visitor::visit(node);
}
