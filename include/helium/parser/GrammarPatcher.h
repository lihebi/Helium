#ifndef GRAMMARPATCHER_H
#define GRAMMARPATCHER_H
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include "helium/parser/visitor.h"

namespace v2 {
  class ASTContext;
  
  class ASTNodeBase;
  class TokenNode;
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


class StandAloneGrammarPatcher {
public:
  StandAloneGrammarPatcher(v2::ASTContext *ast, std::set<v2::ASTNodeBase*> selection)
    : AST(ast), Selection(selection) {}
  ~StandAloneGrammarPatcher() {}
  void process();
  void matchMin(v2::ASTNodeBase *parent, std::set<v2::ASTNodeBase*> sel);
  std::set<v2::ASTNodeBase*> getPatch() {return Patch;}

  bool validAlone(v2::ASTNodeBase* node);
  
private:
  v2::ASTContext *AST = nullptr;
  std::set<v2::ASTNodeBase*> Selection;
  std::set<v2::ASTNodeBase*> Worklist;
  std::set<v2::ASTNodeBase*> Patch;
  std::map<v2::ASTNodeBase*,v2::ASTNodeBase*> Skip;
};


typedef struct PatchData {
  // v2::ASTNodeBase *parent = nullptr;
  std::set<v2::ASTNodeBase*> selection;
  // std::set<v2::ASTNodeBase*> patch;
} PatchData;


/**
 * This is actaully matchMin procedure.
 * Do grammar patching for an AST node and its selected children.
 */
class GrammarPatcher : public Visitor {
public:
  // GrammarPatcher(v2::ASTNodeBase *parent, std::set<v2::ASTNodeBase*> selection)
  //   : parent(parent), selection(selection) {}
  GrammarPatcher() {}
  GrammarPatcher(std::set<v2::ASTNodeBase*> sel) : Selection(sel) {}
  virtual ~GrammarPatcher() {}
  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);

  void setSelection(std::set<v2::ASTNodeBase*> sel) {Selection=sel;}
  
  std::set<v2::ASTNodeBase*> getPatch() {return Patch;}
  void merge(GrammarPatcher *patcher) {
    if (patcher) {
      this->Patch.insert(patcher->Patch.begin(), patcher->Patch.end());
    }
  }
  // std::map<v2::ASTNodeBase*, v2::ASTNodeBase*> getSkip() {return Skip;}
private:
  // std::set<v2::ASTNodeBase*> selection;
  // std::set<v2::ASTNodeBase*> worklist;
  // v2::ASTNodeBase *parent = nullptr;
  // std::set<v2::ASTNodeBase*> selection;
  std::set<v2::ASTNodeBase*> Selection;
  std::set<v2::ASTNodeBase*> Patch;
};

#endif /* GRAMMARPATCHER_H */
