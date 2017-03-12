#ifndef AST_V2_H
#define AST_V2_H

namespace v2 {
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
    TranslationUnit *Unit;
  };


  /**
   * Decls
   */

  
  class Decl {
  public:
    Decl(std::string text) {}
    ~Decl() {}
  };

  class TranslationUnitDecl : Decl {
  public:
    TranslationUnitDecl(std::vector<Decl*> decls,
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
  class DeclStmt : Stmt {
  public:
    DeclStmt() {}
    ~DeclStmt() {}
  };

  /**
   * There's actually no ExprStmt in C grammar, but srcml made it
   */
  class ExprStmt : Stmt {
  public:
    ExprStmt() {}
    ~ExprStmt() {}
  };

  class CompoundStmt : public Stmt {
  public:
    CompoundStmt() {}
    ~CompoundStmt() {}
  };

  class FunctionDecl : public Decl {
  public:
    FunctionDecl(Stmt *body) {}
    ~FunctionDecl() {}
  };

  class ForStmt : public Stmt {
  public:
    ForStmt(Stmt *Init, Expr *Cond, Expr *Inc) {}
    ~ForStmt() {}
  };

  class WhileStmt : public Stmt {
    WhileStmt(Stmt *body, Expr *cond) {}
    ~WhileStmt() {}
  };

  class DoStmt : public Stmt {
  public:
    DoStmt(Stmt *body, Expr *cond) {}
    ~DoStmt() {}
  };

  class IfStmt : public Stmt {
  public:
    IfStmt(Expr *cond, Stmt *body, Stmt *else) {}
    ~IfStmt() {}
  };

  class SwitchStmt : public Stmt {
  public:
    SwitchStmt(Expr *cond, Stmt *body) {}
    ~SwitchStmt() {}
    void AddCase(SwitchCase *case) {}
  };

  /**
   * Base class for CaseStmt and DefaultStmt
   */
  class SwitchCase : public Stmt {
  public:
    SwitchCase() {}
    ~SwitchCase() {}
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
    CaseStmt() {}
    ~CaseStmt() {}
    void Add(Stmt *stmt) {}
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
    Expr() {}
    ~Expr() {}
  };

};

#endif /* AST_V2_H */
