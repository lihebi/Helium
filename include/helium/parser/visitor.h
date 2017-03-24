#ifndef VISITOR_H
#define VISITOR_H

// #include "helium/parser/ast_v2.h"

#include <vector>
#include <map>

namespace v2 {
  class ASTNodeBase;
  class TranslationUnitDecl;
  class FunctionDecl;
  class Decl;
  class Stmt;
  class DeclStmt;
  class ExprStmt;
  class CompoundStmt;
  class ForStmt;
  class WhileStmt;
  class DoStmt;
  class BreakStmt;
  class ContinueStmt;
  class ReturnStmt;
  class IfStmt;
  class SwitchStmt;
  class SwitchCase;
  class CaseStmt;
  class DefaultStmt;
  class Expr;
}

class Visitor {
public:
  Visitor() {}
  virtual ~Visitor() {}
  // virtual void visit(v2::ASTNodeBase *node) = 0;
  virtual void visit(v2::TranslationUnitDecl *unit) = 0;
  virtual void visit(v2::FunctionDecl *function) = 0;
  // virtual void visit(v2::Decl *decl) = 0;
  // virtual void visit(v2::Stmt *stmt) = 0;
  virtual void visit(v2::DeclStmt *decl_stmt) = 0;
  virtual void visit(v2::ExprStmt *expr_stmt) = 0;
  virtual void visit(v2::CompoundStmt *comp_stmt) = 0;
  virtual void visit(v2::ForStmt *for_stmt) = 0;
  virtual void visit(v2::WhileStmt *while_stmt) = 0;
  virtual void visit(v2::DoStmt *do_stmt) = 0;
  virtual void visit(v2::BreakStmt *break_stmt) = 0;
  virtual void visit(v2::ContinueStmt *cont_stmt) = 0;
  virtual void visit(v2::ReturnStmt *ret_stmt) = 0;
  virtual void visit(v2::IfStmt *if_stmt) = 0;
  virtual void visit(v2::SwitchStmt *switch_stmt) = 0;
  // virtual void visit(v2::SwitchCase *switch_case_stmt) = 0;
  virtual void visit(v2::CaseStmt *case_stmt) = 0;
  virtual void visit(v2::DefaultStmt *def_stmt) = 0;
  virtual void visit(v2::Expr *expr) = 0;
};

/**
 * compute the levels
 */
class LevelVisitor : public Visitor {
public:
  LevelVisitor() {}
  ~LevelVisitor() {}

  // virtual void visit(v2::ASTNodeBase *node);
  virtual void visit(v2::TranslationUnitDecl *unit);
  virtual void visit(v2::FunctionDecl *function);
  // virtual void visit(v2::Decl *decl);
  // virtual void visit(v2::Stmt *stmt);
  virtual void visit(v2::DeclStmt *decl_stmt);
  virtual void visit(v2::ExprStmt *expr_stmt);
  virtual void visit(v2::CompoundStmt *comp_stmt);
  virtual void visit(v2::ForStmt *for_stmt);
  virtual void visit(v2::WhileStmt *while_stmt);
  virtual void visit(v2::DoStmt *do_stmt);
  virtual void visit(v2::BreakStmt *break_stmt);
  virtual void visit(v2::ContinueStmt *cont_stmt);
  virtual void visit(v2::ReturnStmt *ret_stmt);
  virtual void visit(v2::IfStmt *if_stmt);
  virtual void visit(v2::SwitchStmt *switch_stmt);
  // virtual void visit(v2::SwitchCase *switch_case_stmt);
  virtual void visit(v2::CaseStmt *case_stmt);
  virtual void visit(v2::DefaultStmt *def_stmt);
  virtual void visit(v2::Expr *expr);

  std::map<v2::ASTNodeBase*, int> getLevels() {return Levels;}

private:
  std::map<v2::ASTNodeBase*, int> Levels;
  int lvl=0;
};

/**
 *  use this to replace the dump method
 */
class Printer : public  Visitor {
public:
  Printer(std::ostream &os) : os(os) {}
  ~Printer() {}

  // virtual void visit(v2::ASTNodeBase *node);
  virtual void visit(v2::TranslationUnitDecl *unit);
  virtual void visit(v2::FunctionDecl *function);
  // virtual void visit(v2::Decl *decl);
  // virtual void visit(v2::Stmt *stmt);
  virtual void visit(v2::DeclStmt *decl_stmt);
  virtual void visit(v2::ExprStmt *expr_stmt);
  virtual void visit(v2::CompoundStmt *comp_stmt);
  virtual void visit(v2::ForStmt *for_stmt);
  virtual void visit(v2::WhileStmt *while_stmt);
  virtual void visit(v2::DoStmt *do_stmt);
  virtual void visit(v2::BreakStmt *break_stmt);
  virtual void visit(v2::ContinueStmt *cont_stmt);
  virtual void visit(v2::ReturnStmt *ret_stmt);
  virtual void visit(v2::IfStmt *if_stmt);
  virtual void visit(v2::SwitchStmt *switch_stmt);
  // virtual void visit(v2::SwitchCase *switch_case_stmt);
  virtual void visit(v2::CaseStmt *case_stmt);
  virtual void visit(v2::DefaultStmt *def_stmt);
  virtual void visit(v2::Expr *expr);

  static std::string PrettyPrint(std::string aststr);
private:
  std::ostream &os;
};

#endif /* VISITOR_H */
