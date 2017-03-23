#ifndef AST_V2_H
#define AST_V2_H

#include <string>
#include <vector>
#include <set>
#include <map>

// #include "helium/parser/benchmark_manager.h"

class BenchmarkManager;

namespace v2 {

  class TranslationUnitDecl;
  class FunctionDecl;
  class Expr;
  class Stmt;
  class Decl;
  class DeclStmt;
  class ASTNodeBase;
  
  /**
   * You can get TranslationUnitDecl from this
   * This also contains the source manager
   * The Loc manager
   */
  class ASTContext {
  public:
    ASTContext(std::string filename) {}
    ~ASTContext() {}
    void setTranslationUnitDecl(TranslationUnitDecl *unit) {
      Unit = unit;
    }
    TranslationUnitDecl *getTranslationUnitDecl() {return Unit;}
    int getLevel(ASTNodeBase *node) {return Levels[node];}
    BenchmarkManager *getBenchmarkManager() {return Manager;}
    std::vector<ASTNodeBase*> getNodes() {return Nodes;}
    /**
     * compute and fill in the Levels variable
     */
    void computeLevels();
    /**
     * traversal and fill the Nodes with pre-order traversal
     */
    void populateNodes();
  private:
    TranslationUnitDecl *Unit = nullptr;
    std::vector<ASTNodeBase*> Nodes;
    std::map<ASTNodeBase*, int> Levels;
    BenchmarkManager *Manager;
    // int level=-1;
    // root will have level 0
    // level will increase
  };


  // I need to travese the AST
  // - so that I can keep the first-order of all nodes into the ASTContext
  // - and also the levels information
  // Or do i need to keep everything in ASTContext?
  // I can just write a visitor to dump nodes

  class ASTNodeBase {
  public:
    ASTNodeBase(ASTContext *ctx) : Ctx(ctx) {}
    ~ASTNodeBase() {}
    virtual std::string label() {return "";}
  protected:
    ASTContext *Ctx = nullptr;
  };

  class TextNode : public ASTNodeBase {
  public:
    TextNode(ASTContext *ctx, std::string text) : ASTNodeBase(ctx), text(text) {}
    ~TextNode() {}
  private:
    std::string text;
  };

  /**
   * Decls
   */

  
  class Decl : public ASTNodeBase {
  public:
    Decl(ASTContext *ctx) : ASTNodeBase(ctx) {}
    ~Decl() {}
    virtual void dump() {}
  };

  class TranslationUnitDecl : public Decl {
  public:
    // TranslationUnitDecl(std::vector<DeclStmt*> decls,
    //                     std::vector<FunctionDecl*> funcs) {}
    TranslationUnitDecl(ASTContext *ctx, std::vector<Decl*> decls) : Decl(ctx), decls(decls) {}
    ~TranslationUnitDecl() {}
    virtual void dump();
  private:
    std::vector<Decl*> decls;
  };


  /**
   * Stmts
   */

  class Stmt : public ASTNodeBase {
  public:
    Stmt(ASTContext *ctx) : ASTNodeBase(ctx) {}
    ~Stmt() {}
    virtual void dump() {}
  };

  /**
   * Adapter class for mixing declarations with statements
   */
  class DeclStmt : public Stmt, public Decl {
  public:
    DeclStmt(ASTContext *ctx, std::string text) : Stmt(ctx), Decl(ctx) {}
    ~DeclStmt() {}
  };

  /**
   * There's actually no ExprStmt in C grammar, but srcml made it
   */
  class ExprStmt : public Stmt {
  public:
    ExprStmt(ASTContext *ctx) : Stmt(ctx) {}
    ~ExprStmt() {}
  };

  class CompoundStmt : public Stmt {
  public:
    CompoundStmt(ASTContext *ctx) : Stmt(ctx) {}
    ~CompoundStmt() {}
    void Add(Stmt *stmt) {
      stmts.push_back(stmt);
    }
  private:
    std::vector<Stmt*> stmts;
  };

  class FunctionDecl : public Decl {
  public:
    FunctionDecl(ASTContext *ctx, std::string name, Stmt *body) : Decl(ctx), name(name), body(body) {}
    ~FunctionDecl() {}
    virtual void dump();
  private:
    std::string name;
    Stmt *body = nullptr;
  };

  class ForStmt : public Stmt {
  public:
    ForStmt(ASTContext *ctx, Expr *Init, Expr *Cond, Expr *Inc, Stmt *body) : Stmt(ctx) {}
    ~ForStmt() {}
  };

  class WhileStmt : public Stmt {
  public:
    WhileStmt(ASTContext *ctx, Expr *cond, Stmt *body) : Stmt(ctx) {}
    ~WhileStmt() {}
  };

  class DoStmt : public Stmt {
  public:
    DoStmt(ASTContext *ctx, Expr *cond, Stmt *body) : Stmt(ctx) {}
    ~DoStmt() {}
  };

  class BreakStmt : public Stmt {
  public:
    BreakStmt(ASTContext *ctx) : Stmt(ctx) {}
    ~BreakStmt() {}
  };
  class ContinueStmt : public Stmt {
  public:
    ContinueStmt(ASTContext *ctx) : Stmt(ctx) {}
    ~ContinueStmt() {}
  };
  class ReturnStmt : public Stmt {
  public:
    ReturnStmt(ASTContext *ctx) : Stmt(ctx) {}
    ~ReturnStmt() {}
  };

  class IfStmt : public Stmt {
  public:
    IfStmt(ASTContext *ctx, Expr *cond, Stmt *thenstmt, Stmt *elsestmt)
      : Stmt(ctx), cond(cond), thenstmt(thenstmt), elsestmt(elsestmt) {}
    ~IfStmt() {}
    void setElse(Stmt *stmt) {
      elsestmt = stmt;
    }
  private:
    Expr *cond;
    Stmt *thenstmt;
    Stmt *elsestmt;
  };

  class SwitchStmt : public Stmt {
  public:
    SwitchStmt(ASTContext *ctx, Expr *cond, Stmt *body) : Stmt(ctx) {}
    ~SwitchStmt() {}
    void AddCase(Stmt *casestmt) {}
  };

  /**
   * Base class for CaseStmt and DefaultStmt
   */
  class SwitchCase : public Stmt {
  public:
    SwitchCase(ASTContext *ctx) : Stmt(ctx) {}
    ~SwitchCase() {}
    void Add(Stmt *stmt) {}
  };

  /**
   * This is tricky. The grammar says:
   * labeled-statement ::= case : statement
   * But this is a single statement.
   * The grammar actually permits 0 or more statements without curly braces to make them compound statement
   * Thus, we need to make some decision here:
   * The easiest way seems to be alter the grammar to be
   * labeled-statement ::= case : [case-body-stmt]_opt
   * case-body-stmt ::= stmt case-body-stmt
   * Or using eBNF
   * labeled-statement ::= case : [stmt]*
   * The implementation is trivial: use a vector of statements
   */
  class CaseStmt : public SwitchCase {
  public:
    CaseStmt(ASTContext *ctx, Expr *cond) : SwitchCase(ctx) {}
    ~CaseStmt() {}
  };

  class DefaultStmt : public SwitchCase {
  public:
    DefaultStmt(ASTContext *ctx) : SwitchCase(ctx) {}
    ~DefaultStmt() {}
  };

  /**
   * Exprs
   */

  class Expr {
  public:
    Expr(std::string text) {}
    ~Expr() {}
  };

}
#endif /* AST_V2_H */
