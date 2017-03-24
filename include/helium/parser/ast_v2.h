#ifndef AST_V2_H
#define AST_V2_H

#include <string>
#include <vector>
#include <set>
#include <map>

#include <ostream>

#include "helium/parser/visitor.h"
// class Visitor;

// #include "helium/parser/source_manager.h"

class SourceManager;

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
    void setSourceManager(SourceManager *manager) {Manager=manager;}
    SourceManager *getSourceManager() {return Manager;}
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
    SourceManager *Manager = nullptr;
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
    // virtual void accept(Visitor *visitor) {
    //   visitor->visit(this);
    // }
    virtual void accept(Visitor *visitor) = 0;
    ASTContext *getASTContext() {return Ctx;}
  protected:
    ASTContext *Ctx = nullptr;
  };

  // class TextNode : public ASTNodeBase {
  // public:
  //   TextNode(ASTContext *ctx, std::string text) : ASTNodeBase(ctx), text(text) {}
  //   ~TextNode() {}
  // private:
  //   std::string text;
  // };

  /**
   * Decls
   */

  
  class Decl : public ASTNodeBase {
  public:
    Decl(ASTContext *ctx) : ASTNodeBase(ctx) {}
    ~Decl() {}
  };

  class TranslationUnitDecl : public Decl {
  public:
    // TranslationUnitDecl(std::vector<DeclStmt*> decls,
    //                     std::vector<FunctionDecl*> funcs) {}
    TranslationUnitDecl(ASTContext *ctx, std::vector<ASTNodeBase*> decls) : Decl(ctx), decls(decls) {}
    ~TranslationUnitDecl() {}
    std::vector<ASTNodeBase*> getDecls() {return decls;}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  private:
    std::vector<ASTNodeBase*> decls;
  };


  /**
   * Stmts
   */

  class Stmt : public ASTNodeBase {
  public:
    Stmt(ASTContext *ctx) : ASTNodeBase(ctx) {}
    ~Stmt() {}
  };

  /**
   * Adapter class for mixing declarations with statements
   */
  class DeclStmt : public Stmt {
  public:
    DeclStmt(ASTContext *ctx, std::string text) : Stmt(ctx) {}
    ~DeclStmt() {}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  };

  /**
   * There's actually no ExprStmt in C grammar, but srcml made it
   */
  class ExprStmt : public Stmt {
  public:
    ExprStmt(ASTContext *ctx) : Stmt(ctx) {}
    ~ExprStmt() {}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  };

  class CompoundStmt : public Stmt {
  public:
    CompoundStmt(ASTContext *ctx) : Stmt(ctx) {}
    ~CompoundStmt() {}
    void Add(Stmt *stmt) {
      stmts.push_back(stmt);
    }
    std::vector<Stmt*> getBody() {return stmts;}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  private:
    std::vector<Stmt*> stmts;
  };

  class FunctionDecl : public Decl {
  public:
    FunctionDecl(ASTContext *ctx, std::string name, Stmt *body) : Decl(ctx), name(name), body(body) {}
    ~FunctionDecl() {}
    Stmt *getBody() {return body;}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
    std::string getName() {return name;}
  private:
    std::string name;
    Stmt *body = nullptr;
  };

  class ForStmt : public Stmt {
  public:
    ForStmt(ASTContext *ctx, Expr *Init, Expr *Cond, Expr *Inc, Stmt *Body)
      : Stmt(ctx), Init(Init), Cond(Cond), Inc(Inc), Body(Body) {}
    ~ForStmt() {}
    Expr *getInit() {return Init;}
    Expr *getCond() {return Cond;}
    Expr *getInc() {return Inc;}
    Stmt *getBody() {return Body;}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  private:
    Expr *Init = nullptr;
    Expr *Cond = nullptr;
    Expr *Inc = nullptr;
    Stmt *Body = nullptr;
  };

  class WhileStmt : public Stmt {
  public:
    WhileStmt(ASTContext *ctx, Expr *cond, Stmt *Body)
      : Stmt(ctx), Cond(cond), Body(Body) {}
    ~WhileStmt() {}
    Expr *getCond() {return Cond;}
    Stmt *getBody() {return Body;}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  private:
    Expr *Cond;
    Stmt *Body;
  };

  class DoStmt : public Stmt {
  public:
    DoStmt(ASTContext *ctx, Expr *cond, Stmt *body)
      : Stmt(ctx), Cond(cond), Body(body) {}
    ~DoStmt() {}
    Expr *getCond() {return Cond;}
    Stmt *getBody() {return Body;}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  private:
    Expr *Cond;
    Stmt *Body;
  };

  class BreakStmt : public Stmt {
  public:
    BreakStmt(ASTContext *ctx) : Stmt(ctx) {}
    ~BreakStmt() {}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  };
  class ContinueStmt : public Stmt {
  public:
    ContinueStmt(ASTContext *ctx) : Stmt(ctx) {}
    ~ContinueStmt() {}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  };
  class ReturnStmt : public Stmt {
  public:
    ReturnStmt(ASTContext *ctx) : Stmt(ctx) {}
    ~ReturnStmt() {}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  };

  class IfStmt : public Stmt {
  public:
    IfStmt(ASTContext *ctx, Expr *cond, Stmt *thenstmt, Stmt *elsestmt)
      : Stmt(ctx), cond(cond), thenstmt(thenstmt), elsestmt(elsestmt) {}
    ~IfStmt() {}
    void setElse(Stmt *stmt) {
      elsestmt = stmt;
    }
    Expr *getCond() {return cond;}
    Stmt *getThen() {return thenstmt;}
    Stmt *getElse() {return elsestmt;}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  private:
    Expr *cond;
    Stmt *thenstmt;
    Stmt *elsestmt;
  };

  class SwitchStmt : public Stmt {
  public:
    // FIXME body??
    SwitchStmt(ASTContext *ctx, Expr *cond, Stmt *body)
      : Stmt(ctx), Cond(cond) {}
    ~SwitchStmt() {}
    void AddCase(SwitchCase *casestmt) {Cases.push_back(casestmt);}
    std::vector<SwitchCase*> getCases() {return Cases;}
    Expr *getCond() {return Cond;}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  private:
    Expr *Cond = nullptr;
    std::vector<SwitchCase*> Cases;
  };

  /**
   * Base class for CaseStmt and DefaultStmt
   */
  class SwitchCase : public Stmt {
  public:
    SwitchCase(ASTContext *ctx) : Stmt(ctx) {}
    ~SwitchCase() {}
    void Add(Stmt *stmt) {Body.push_back(stmt);}
    std::vector<Stmt*> getBody() {return Body;}
  protected:
    std::vector<Stmt*> Body;
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
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  };

  class DefaultStmt : public SwitchCase {
  public:
    DefaultStmt(ASTContext *ctx) : SwitchCase(ctx) {}
    ~DefaultStmt() {}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  };

  /**
   * Exprs
   */

  class Expr : public ASTNodeBase {
  public:
    Expr(ASTContext *ctx, std::string text) : ASTNodeBase(ctx) {}
    ~Expr() {}
    virtual void accept(Visitor *visitor) {
      visitor->visit(this);
    }
  };

}
#endif /* AST_V2_H */
