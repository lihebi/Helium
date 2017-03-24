#ifndef VISITOR_H
#define VISITOR_H

// #include "helium/parser/ast_v2.h"

#include <vector>
#include <map>
#include <set>


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


/**
 * \defgroup visitor
 * \ingroup parser
 * Visitors for AST
 */

/**
 * \ingroup visitor
 *
 * visitor interface
 */
class Visitor {
public:
  Visitor() {}
  virtual ~Visitor() {}
  // virtual void visit(v2::ASTNodeBase *node) = 0;
  virtual void visit(v2::TokenNode *token, void *data=nullptr) = 0;
  virtual void visit(v2::TranslationUnitDecl *unit, void *data=nullptr) = 0;
  virtual void visit(v2::FunctionDecl *function, void *data=nullptr) = 0;
  // virtual void visit(v2::Decl *decl) = 0;
  // virtual void visit(v2::Stmt *stmt) = 0;
  virtual void visit(v2::DeclStmt *decl_stmt, void *data=nullptr) = 0;
  virtual void visit(v2::ExprStmt *expr_stmt, void *data=nullptr) = 0;
  virtual void visit(v2::CompoundStmt *comp_stmt, void *data=nullptr) = 0;
  virtual void visit(v2::ForStmt *for_stmt, void *data=nullptr) = 0;
  virtual void visit(v2::WhileStmt *while_stmt, void *data=nullptr) = 0;
  virtual void visit(v2::DoStmt *do_stmt, void *data=nullptr) = 0;
  virtual void visit(v2::BreakStmt *break_stmt, void *data=nullptr) = 0;
  virtual void visit(v2::ContinueStmt *cont_stmt, void *data=nullptr) = 0;
  virtual void visit(v2::ReturnStmt *ret_stmt, void *data=nullptr) = 0;
  virtual void visit(v2::IfStmt *if_stmt, void *data=nullptr) = 0;
  virtual void visit(v2::SwitchStmt *switch_stmt, void *data=nullptr) = 0;
  // virtual void visit(v2::SwitchCase *switch_case_stmt) = 0;
  virtual void visit(v2::CaseStmt *case_stmt, void *data=nullptr) = 0;
  virtual void visit(v2::DefaultStmt *def_stmt, void *data=nullptr) = 0;
  virtual void visit(v2::Expr *expr, void *data=nullptr) = 0;
};

/**
 * compute the levels
 *
 * \ingroup visitor
 */
class LevelVisitor : public Visitor {
public:
  LevelVisitor() {}
  ~LevelVisitor() {}

  // virtual void visit(v2::ASTNodeBase *node);
  virtual void visit(v2::TokenNode *token, void *data=nullptr);
  virtual void visit(v2::TranslationUnitDecl *unit, void *data=nullptr);
  virtual void visit(v2::FunctionDecl *function, void *data=nullptr);
  // virtual void visit(v2::Decl *decl);
  // virtual void visit(v2::Stmt *stmt);
  virtual void visit(v2::DeclStmt *decl_stmt, void *data=nullptr);
  virtual void visit(v2::ExprStmt *expr_stmt, void *data=nullptr);
  virtual void visit(v2::CompoundStmt *comp_stmt, void *data=nullptr);
  virtual void visit(v2::ForStmt *for_stmt, void *data=nullptr);
  virtual void visit(v2::WhileStmt *while_stmt, void *data=nullptr);
  virtual void visit(v2::DoStmt *do_stmt, void *data=nullptr);
  virtual void visit(v2::BreakStmt *break_stmt, void *data=nullptr);
  virtual void visit(v2::ContinueStmt *cont_stmt, void *data=nullptr);
  virtual void visit(v2::ReturnStmt *ret_stmt, void *data=nullptr);
  virtual void visit(v2::IfStmt *if_stmt, void *data=nullptr);
  virtual void visit(v2::SwitchStmt *switch_stmt, void *data=nullptr);
  // virtual void visit(v2::SwitchCase *switch_case_stmt);
  virtual void visit(v2::CaseStmt *case_stmt, void *data=nullptr);
  virtual void visit(v2::DefaultStmt *def_stmt, void *data=nullptr);
  virtual void visit(v2::Expr *expr, void *data=nullptr);

  std::map<v2::ASTNodeBase*, int> getLevels() {return Levels;}
  int getLevel(v2::ASTNodeBase *node) {
    if (Levels.count(node) == 1) {
      return Levels[node];
    } else {
      return -1;
    }
  }
  v2::ASTNodeBase *getLowestLevelNode(std::set<v2::ASTNodeBase*> nodes) {
    int retlvl=-1;
    v2::ASTNodeBase *ret = nullptr;
    for (auto *node : nodes) {
      int lvl = getLevel(node);
      if (retlvl < lvl) {
        retlvl = lvl;
        ret = node;
      }
    }
    return ret;
  }

private:
  std::map<v2::ASTNodeBase*, int> Levels;
  int lvl=0;
};

/**
 * \ingroup visitor
 *  use this to replace the dump method
 */
class Printer : public  Visitor {
public:
  Printer(std::ostream &os) : os(os) {}
  ~Printer() {}

  virtual void visit(v2::TokenNode *token, void *data=nullptr);
  // virtual void visit(v2::ASTNodeBase *node);
  virtual void visit(v2::TranslationUnitDecl *unit, void *data=nullptr);
  virtual void visit(v2::FunctionDecl *function, void *data=nullptr);
  // virtual void visit(v2::Decl *decl);
  // virtual void visit(v2::Stmt *stmt);
  virtual void visit(v2::DeclStmt *decl_stmt, void *data=nullptr);
  virtual void visit(v2::ExprStmt *expr_stmt, void *data=nullptr);
  virtual void visit(v2::CompoundStmt *comp_stmt, void *data=nullptr);
  virtual void visit(v2::ForStmt *for_stmt, void *data=nullptr);
  virtual void visit(v2::WhileStmt *while_stmt, void *data=nullptr);
  virtual void visit(v2::DoStmt *do_stmt, void *data=nullptr);
  virtual void visit(v2::BreakStmt *break_stmt, void *data=nullptr);
  virtual void visit(v2::ContinueStmt *cont_stmt, void *data=nullptr);
  virtual void visit(v2::ReturnStmt *ret_stmt, void *data=nullptr);
  virtual void visit(v2::IfStmt *if_stmt, void *data=nullptr);
  virtual void visit(v2::SwitchStmt *switch_stmt, void *data=nullptr);
  // virtual void visit(v2::SwitchCase *switch_case_stmt);
  virtual void visit(v2::CaseStmt *case_stmt, void *data=nullptr);
  virtual void visit(v2::DefaultStmt *def_stmt, void *data=nullptr);
  virtual void visit(v2::Expr *expr, void *data=nullptr);

  static std::string PrettyPrint(std::string aststr);
private:
  std::ostream &os;
};

/**
 * Token here means the leaf-node of AST, so it is abstract token.
 * The granularity depends on the parser precision.
 */
class TokenVisitor : public Visitor {
public:
  TokenVisitor() {}
  ~TokenVisitor() {}
  virtual void visit(v2::TokenNode *token, void *data=nullptr);
  virtual void visit(v2::TranslationUnitDecl *unit, void *data=nullptr);
  virtual void visit(v2::FunctionDecl *function, void *data=nullptr);
  virtual void visit(v2::DeclStmt *decl_stmt, void *data=nullptr);
  virtual void visit(v2::ExprStmt *expr_stmt, void *data=nullptr);
  virtual void visit(v2::CompoundStmt *comp_stmt, void *data=nullptr);
  virtual void visit(v2::ForStmt *for_stmt, void *data=nullptr);
  virtual void visit(v2::WhileStmt *while_stmt, void *data=nullptr);
  virtual void visit(v2::DoStmt *do_stmt, void *data=nullptr);
  virtual void visit(v2::BreakStmt *break_stmt, void *data=nullptr);
  virtual void visit(v2::ContinueStmt *cont_stmt, void *data=nullptr);
  virtual void visit(v2::ReturnStmt *ret_stmt, void *data=nullptr);
  virtual void visit(v2::IfStmt *if_stmt, void *data=nullptr);
  virtual void visit(v2::SwitchStmt *switch_stmt, void *data=nullptr);
  virtual void visit(v2::CaseStmt *case_stmt, void *data=nullptr);
  virtual void visit(v2::DefaultStmt *def_stmt, void *data=nullptr);
  virtual void visit(v2::Expr *expr, void *data=nullptr);
  std::vector<v2::ASTNodeBase*> getTokens() {return Tokens;}
  std::map<v2::ASTNodeBase*,int> getIdMap() {return IdMap;}
  int getId(v2::ASTNodeBase *node) {
    if (IdMap.count(node) == 1) return IdMap[node];
    return -1;
  }
private:
  int id = 0;
  std::map<v2::ASTNodeBase*,int> IdMap; // ID start from 0
  std::vector<v2::ASTNodeBase*> Tokens; // this is actually ID->Node implemented in vector
};

/**
 * Build a reverse index from children node to its parent
 */
class ParentIndexer : public Visitor {
public:
  ParentIndexer() {}
  ~ParentIndexer() {}
  std::map<v2::ASTNodeBase*, v2::ASTNodeBase*> getParentMap() {return ParentMap;}
  virtual void visit(v2::TokenNode *token, void *data=nullptr);
  virtual void visit(v2::TranslationUnitDecl *unit, void *data=nullptr);
  virtual void visit(v2::FunctionDecl *function, void *data=nullptr);
  virtual void visit(v2::DeclStmt *decl_stmt, void *data=nullptr);
  virtual void visit(v2::ExprStmt *expr_stmt, void *data=nullptr);
  virtual void visit(v2::CompoundStmt *comp_stmt, void *data=nullptr);
  virtual void visit(v2::ForStmt *for_stmt, void *data=nullptr);
  virtual void visit(v2::WhileStmt *while_stmt, void *data=nullptr);
  virtual void visit(v2::DoStmt *do_stmt, void *data=nullptr);
  virtual void visit(v2::BreakStmt *break_stmt, void *data=nullptr);
  virtual void visit(v2::ContinueStmt *cont_stmt, void *data=nullptr);
  virtual void visit(v2::ReturnStmt *ret_stmt, void *data=nullptr);
  virtual void visit(v2::IfStmt *if_stmt, void *data=nullptr);
  virtual void visit(v2::SwitchStmt *switch_stmt, void *data=nullptr);
  virtual void visit(v2::CaseStmt *case_stmt, void *data=nullptr);
  virtual void visit(v2::DefaultStmt *def_stmt, void *data=nullptr);
  virtual void visit(v2::Expr *expr, void *data=nullptr);

  std::set<v2::ASTNodeBase*> getChildren(v2::ASTNodeBase* parent) {
    if (ChildrenMap.count(parent) == 1) {
      return ChildrenMap[parent];
    }
    return std::set<v2::ASTNodeBase*>();
  }

  v2::ASTNodeBase *getParent(v2::ASTNodeBase *node) {
    if (ParentMap.count(node) == 1) return ParentMap[node];
    return nullptr;
  }
private:
  std::map<v2::ASTNodeBase*, v2::ASTNodeBase*> ParentMap;
  std::map<v2::ASTNodeBase*, std::set<v2::ASTNodeBase*> > ChildrenMap;
};


class StandAloneGrammarPatcher {
public:
  StandAloneGrammarPatcher(v2::ASTContext *ast, std::set<v2::ASTNodeBase*> selection)
    : AST(ast), selection(selection) {}
  ~StandAloneGrammarPatcher() {}
  void process();
  std::set<v2::ASTNodeBase*> matchMin(v2::ASTNodeBase *parent, std::set<v2::ASTNodeBase*> sel);
  std::set<v2::ASTNodeBase*> getPatch() {return patch;}
private:
  v2::ASTContext *AST = nullptr;
  std::set<v2::ASTNodeBase*> selection;
  std::set<v2::ASTNodeBase*> worklist;
  std::set<v2::ASTNodeBase*> patch;
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
  ~GrammarPatcher() {}
  virtual void visit(v2::TokenNode *token, void *data=nullptr);
  virtual void visit(v2::TranslationUnitDecl *unit, void *data=nullptr);
  virtual void visit(v2::FunctionDecl *function, void *data=nullptr);
  virtual void visit(v2::DeclStmt *decl_stmt, void *data=nullptr);
  virtual void visit(v2::ExprStmt *expr_stmt, void *data=nullptr);
  virtual void visit(v2::CompoundStmt *comp_stmt, void *data=nullptr);
  virtual void visit(v2::ForStmt *for_stmt, void *data=nullptr);
  virtual void visit(v2::WhileStmt *while_stmt, void *data=nullptr);
  virtual void visit(v2::DoStmt *do_stmt, void *data=nullptr);
  virtual void visit(v2::BreakStmt *break_stmt, void *data=nullptr);
  virtual void visit(v2::ContinueStmt *cont_stmt, void *data=nullptr);
  virtual void visit(v2::ReturnStmt *ret_stmt, void *data=nullptr);
  virtual void visit(v2::IfStmt *if_stmt, void *data=nullptr);
  virtual void visit(v2::SwitchStmt *switch_stmt, void *data=nullptr);
  virtual void visit(v2::CaseStmt *case_stmt, void *data=nullptr);
  virtual void visit(v2::DefaultStmt *def_stmt, void *data=nullptr);
  virtual void visit(v2::Expr *expr, void *data=nullptr);
  std::set<v2::ASTNodeBase*> getPatch() {return patch;}
private:
  // std::set<v2::ASTNodeBase*> selection;
  // std::set<v2::ASTNodeBase*> worklist;
  // v2::ASTNodeBase *parent = nullptr;
  // std::set<v2::ASTNodeBase*> selection;
  std::set<v2::ASTNodeBase*> patch;
};

#endif /* VISITOR_H */
