#ifndef AST_V2_H
#define AST_V2_H

#include <string>
#include <vector>


namespace v2 {

  class TranslationUnitDecl;
  class FunctionDecl;
  class Expr;
  class Stmt;
  class Decl;
  class DeclStmt;
  
  /**
   * You can get TranslationUnitDecl from this
   * This also contains the source manager
   * The Loc manager
   */
  class ASTContext {
  public:
    ASTContext(std::string filename) {}
    ~ASTContext() {}
    TranslationUnitDecl *getTranslationUnitDecl() {return Unit;}
  private:
    TranslationUnitDecl *Unit;
  };


  /**
   * Decls
   */

  
  class Decl {
  public:
    Decl() {}
    ~Decl() {}
  };

  class TranslationUnitDecl : public Decl {
  public:
    TranslationUnitDecl(std::vector<DeclStmt*> decls,
                        std::vector<FunctionDecl*> funcs) {}
    ~TranslationUnitDecl() {}
  };


  /**
   * Stmts
   */

  class Stmt {
  public:
    Stmt() {}
    ~Stmt() {}
  };

  /**
   * Adapter class for mixing declarations with statements
   */
  class DeclStmt : public Stmt {
  public:
    DeclStmt(std::string text) {}
    ~DeclStmt() {}
  };

  /**
   * There's actually no ExprStmt in C grammar, but srcml made it
   */
  class ExprStmt : public Stmt {
  public:
    ExprStmt() {}
    ~ExprStmt() {}
  };

  class CompoundStmt : public Stmt {
  public:
    CompoundStmt() {}
    ~CompoundStmt() {}
    void Add(Stmt *stmt) {
      stmts.push_back(stmt);
    }
  private:
    std::vector<Stmt*> stmts;
  };

  class FunctionDecl : public Decl {
  public:
    FunctionDecl(Stmt *body) {}
    ~FunctionDecl() {}
  };

  class ForStmt : public Stmt {
  public:
    ForStmt(Expr *Init, Expr *Cond, Expr *Inc, Stmt *body) {}
    ~ForStmt() {}
  };

  class WhileStmt : public Stmt {
  public:
    WhileStmt(Expr *cond, Stmt *body) {}
    ~WhileStmt() {}
  };

  class DoStmt : public Stmt {
  public:
    DoStmt(Expr *cond, Stmt *body) {}
    ~DoStmt() {}
  };

  class BreakStmt : public Stmt {
  public:
    BreakStmt() {}
    ~BreakStmt() {}
  };
  class ContinueStmt : public Stmt {
  public:
    ContinueStmt() {}
    ~ContinueStmt() {}
  };
  class ReturnStmt : public Stmt {
  public:
    ReturnStmt() {}
    ~ReturnStmt() {}
  };

  class IfStmt : public Stmt {
  public:
    IfStmt(Expr *cond, Stmt *thenstmt, Stmt *elsestmt)
      : cond(cond), thenstmt(thenstmt), elsestmt(elsestmt) {}
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
    SwitchStmt(Expr *cond, Stmt *body) {}
    ~SwitchStmt() {}
    void AddCase(Stmt *casestmt) {}
  };

  /**
   * Base class for CaseStmt and DefaultStmt
   */
  class SwitchCase : public Stmt {
  public:
    SwitchCase() {}
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
    CaseStmt(Expr *cond) {}
    ~CaseStmt() {}
  };

  class DefaultStmt : public SwitchCase {
  public:
    DefaultStmt();
    ~DefaultStmt();
  };

  /**
   * Exprs
   */

  class Expr {
  public:
    Expr(std::string text) {}
    ~Expr() {}
  };

};

#endif /* AST_V2_H */
