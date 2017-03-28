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

#include "helium/parser/source_location.h"

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
   * \addtogroup ast
   * \ingroup parser
   *
   * ast group is a subgroup of parser group
   * @{
   */

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
    ASTNodeBase(ASTContext *ctx, SourceLocation begin, SourceLocation end)
      : Ctx(ctx), BeginLoc(begin), EndLoc(end)  {}
    ~ASTNodeBase() {}
    virtual std::string label() {return "";}
    // virtual void accept(Visitor *visitor) {
    //   visitor->visit(this);
    // }
    virtual void accept(Visitor *visitor, void *data=nullptr) = 0;
    ASTContext *getASTContext() {return Ctx;}
    SourceLocation getBeginLoc() {return BeginLoc;}
    SourceLocation getEndLoc() {return EndLoc;}
    virtual void dump(std::ostream &os) = 0;
  protected:
    ASTContext *Ctx = nullptr;
    SourceLocation BeginLoc;
    SourceLocation EndLoc;
  };

  class TokenNode : public ASTNodeBase {
  public:
    TokenNode(ASTContext *ctx, std::string text, SourceLocation begin, SourceLocation end)
      : ASTNodeBase(ctx, begin, end), Text(text) {}
    TokenNode(ASTContext *ctx, std::string text)
      : ASTNodeBase(ctx, SourceLocation(-1,-1), SourceLocation(-1,-1)) {}
    ~TokenNode() {}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    std::string getText() {return Text;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << " TokenNode: " << Text << ")";
    }
  private:
    std::string Text;
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
    Decl(ASTContext *ctx, SourceLocation begin, SourceLocation end)
      : ASTNodeBase(ctx, begin, end) {}
    ~Decl() {}
  };

  class TranslationUnitDecl : public Decl {
  public:
    // TranslationUnitDecl(std::vector<DeclStmt*> decls,
    //                     std::vector<FunctionDecl*> funcs) {}
    TranslationUnitDecl(ASTContext *ctx, std::vector<ASTNodeBase*> decls,
                        SourceLocation begin, SourceLocation end)
      : Decl(ctx, begin, end), decls(decls) {}
    ~TranslationUnitDecl() {}
    std::vector<ASTNodeBase*> getDecls() {return decls;}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << "TranslationUnitDecl)";
    }
  private:
    std::vector<ASTNodeBase*> decls;
  };


  /**
   * Stmts
   */

  class Stmt : public ASTNodeBase {
  public:
    Stmt(ASTContext *ctx, SourceLocation begin, SourceLocation end)
      : ASTNodeBase(ctx, begin, end) {}
    ~Stmt() {}
  };

  /**
   * Adapter class for mixing declarations with statements
   */
  class DeclStmt : public Stmt {
  public:
    DeclStmt(ASTContext *ctx, std::string text, SourceLocation begin, SourceLocation end)
      : Stmt(ctx, begin, end), Text(text) {}
    ~DeclStmt() {}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    std::string getText() {return Text;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << "DeclStmt)";
    }
  private:
    std::string Text;
  };

  /**
   * There's actually no ExprStmt in C grammar, but srcml made it
   */
  class ExprStmt : public Stmt {
  public:
    ExprStmt(ASTContext *ctx, std::string text, SourceLocation begin, SourceLocation end)
      : Stmt(ctx, begin, end), Text(text) {}
    ~ExprStmt() {}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    std::string getText() {return Text;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << "ExprStmt)";
    }
  private:
    std::string Text;
  };

  class CompoundStmt : public Stmt {
  public:
    CompoundStmt(ASTContext *ctx, SourceLocation begin, SourceLocation end)
      : Stmt(ctx, begin, end), CompNode(new TokenNode(ctx, "COMP_DUMMY")) {}
    ~CompoundStmt() {}
    void Add(Stmt *stmt) {
      stmts.push_back(stmt);
    }
    std::vector<Stmt*> getBody() {return stmts;}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }

    TokenNode *getCompNode() {return CompNode;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << "CompStmt)";
    }
  private:
    // signature node. Even if compound statement do not have a
    // keyword, I need one to record if I select it or not
    // This is very useful to make the braces correct
    TokenNode *CompNode = nullptr;
    std::vector<Stmt*> stmts;
  };

  class FunctionDecl : public Decl {
  public:
    FunctionDecl(ASTContext *ctx, std::string name, Stmt *body, SourceLocation begin, SourceLocation end)
      : Decl(ctx, begin, end), name(name), body(body) {
      // TODO populate the three token nodes
    }
    ~FunctionDecl() {}
    Stmt *getBody() {return body;}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    std::string getName() {return name;}
    TokenNode *getReturnTypeNode() {return ReturnTypeNode;}
    TokenNode *getNameNode() {return NameNode;}
    TokenNode *getParamNode() {return ParamNode;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << "FunctionDecl)";
    }
  private:
    std::string name;
    Stmt *body = nullptr;
    TokenNode *ReturnTypeNode = nullptr;
    TokenNode *NameNode = nullptr;
    // not sure if param node is useful
    // (HEBI: param, although is omitable, should never be ommited)
    // (HEBI: what if selection is across multiple function?? Which as main??)
    // (HEBI: what if selection is across multiple files? There will be multiple translation unit, aka mulitple ASTs)
    TokenNode *ParamNode = nullptr;
  };

  class ForStmt : public Stmt {
  public:
    ForStmt(ASTContext *ctx, Expr *Init, Expr *Cond, Expr *Inc, Stmt *Body, TokenNode *ForNode,
            SourceLocation begin, SourceLocation end)
      : Stmt(ctx, begin, end), Init(Init), Cond(Cond), Inc(Inc), Body(Body), ForNode(ForNode) {
    }
    ~ForStmt() {}
    Expr *getInit() {return Init;}
    Expr *getCond() {return Cond;}
    Expr *getInc() {return Inc;}
    Stmt *getBody() {return Body;}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    TokenNode *getForNode() {return ForNode;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << "ForStmt)";
    }
  private:
    Expr *Init = nullptr;
    Expr *Cond = nullptr;
    Expr *Inc = nullptr;
    Stmt *Body = nullptr;
    TokenNode *ForNode = nullptr;
  };

  class WhileStmt : public Stmt {
  public:
    WhileStmt(ASTContext *ctx, Expr *cond, Stmt *Body, TokenNode *WhileNode,
              SourceLocation begin, SourceLocation end)
      : Stmt(ctx, begin, end), Cond(cond), Body(Body), WhileNode(WhileNode) {
    }
    ~WhileStmt() {}
    Expr *getCond() {return Cond;}
    Stmt *getBody() {return Body;}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    TokenNode *getWhileNode() {return WhileNode;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << "WhileStmt)";
    }
  private:
    Expr *Cond;
    Stmt *Body;
    TokenNode *WhileNode = nullptr;
  };

  class DoStmt : public Stmt {
  public:
    DoStmt(ASTContext *ctx, Expr *cond, Stmt *body, TokenNode *DoNode, TokenNode *WhileNode,
           SourceLocation begin, SourceLocation end)
      : Stmt(ctx, begin, end), Cond(cond), Body(body), DoNode(DoNode), WhileNode(WhileNode) {}
    ~DoStmt() {}
    Expr *getCond() {return Cond;}
    Stmt *getBody() {return Body;}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    TokenNode *getDoNode() {return DoNode;}
    TokenNode *getWhileNode() {return WhileNode;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << "DoStmt)";
    }
  private:
    Expr *Cond;
    Stmt *Body;
    TokenNode *DoNode = nullptr;
    TokenNode *WhileNode = nullptr;
  };

  class BreakStmt : public Stmt {
  public:
    BreakStmt(ASTContext *ctx, SourceLocation begin, SourceLocation end) : Stmt(ctx, begin, end) {}
    ~BreakStmt() {}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << " BreakStmt)";
    }
  };
  class ContinueStmt : public Stmt {
  public:
    ContinueStmt(ASTContext *ctx, SourceLocation begin, SourceLocation end) : Stmt(ctx, begin, end) {}
    ~ContinueStmt() {}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << " ContinueStmt)";
    }
  };
  class ReturnStmt : public Stmt {
  public:
    ReturnStmt(ASTContext *ctx, TokenNode *ReturnNode, SourceLocation begin, SourceLocation end)
      : Stmt(ctx, begin, end), ReturnNode(ReturnNode) {
    }
    ~ReturnStmt() {}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    Expr *getValue() {return Value;}
    TokenNode *getReturnNode() {return ReturnNode;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << " ReturnStmt)";
    }
  private:
    Expr *Value = nullptr;
    TokenNode *ReturnNode = nullptr;
  };

  class IfStmt : public Stmt {
  public:
    IfStmt(ASTContext *ctx, Expr *cond, Stmt *thenstmt, Stmt *elsestmt,
           TokenNode *IfNode, TokenNode *ElseNode,
           SourceLocation begin, SourceLocation end)
      : Stmt(ctx, begin, end), cond(cond), thenstmt(thenstmt), elsestmt(elsestmt),
        IfNode(IfNode), ElseNode(ElseNode) {
    }
    ~IfStmt() {}
    void setElse(Stmt *stmt) {
      elsestmt = stmt;
    }
    Expr *getCond() {return cond;}
    Stmt *getThen() {return thenstmt;}
    Stmt *getElse() {return elsestmt;}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    TokenNode *getIfNode() {return IfNode;}
    TokenNode *getElseNode() {return ElseNode;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << " IfStmt)";
    }
  private:
    Expr *cond = nullptr;
    Stmt *thenstmt = nullptr;
    Stmt *elsestmt = nullptr;
    TokenNode *IfNode = nullptr;
    TokenNode *ElseNode = nullptr;
  };

  class SwitchStmt : public Stmt {
  public:
    // FIXME body??
    SwitchStmt(ASTContext *ctx, Expr *cond, Stmt *body, TokenNode *SwitchNode,
               SourceLocation begin, SourceLocation end)
      : Stmt(ctx, begin, end), Cond(cond), SwitchNode(SwitchNode) {
    }
    ~SwitchStmt() {}
    void AddCase(SwitchCase *casestmt) {Cases.push_back(casestmt);}
    std::vector<SwitchCase*> getCases() {return Cases;}
    Expr *getCond() {return Cond;}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    TokenNode *getSwitchNode() {return SwitchNode;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << " SwitchStmt)";
    }
  private:
    Expr *Cond = nullptr;
    std::vector<SwitchCase*> Cases;
    TokenNode *SwitchNode = nullptr;
  };

  /**
   * Base class for CaseStmt and DefaultStmt
   */
  class SwitchCase : public Stmt {
  public:
    SwitchCase(ASTContext *ctx, SourceLocation begin, SourceLocation end) : Stmt(ctx, begin, end) {}
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
    CaseStmt(ASTContext *ctx, Expr *cond, TokenNode *CaseNode, SourceLocation begin, SourceLocation end)
      : SwitchCase(ctx, begin, end), Cond(cond), CaseNode(CaseNode) {
    }
    ~CaseStmt() {}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    TokenNode *getCaseNode() {return CaseNode;}
    Expr *getCond() {return Cond;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << " CaseStmt)";
    }
  private:
    Expr *Cond = nullptr;
    TokenNode *CaseNode = nullptr;
  };

  class DefaultStmt : public SwitchCase {
  public:
    DefaultStmt(ASTContext *ctx, TokenNode *DefaultNode, SourceLocation begin, SourceLocation end)
      : SwitchCase(ctx, begin, end), DefaultNode(DefaultNode) {
    }
    ~DefaultStmt() {}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    TokenNode *getDefaultNode() {return DefaultNode;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << " DefaultStmt)";
    }
  private:
    TokenNode *DefaultNode = nullptr;
  };

  /**
   * Exprs
   */

  class Expr : public ASTNodeBase {
  public:
    Expr(ASTContext *ctx, std::string text, SourceLocation begin, SourceLocation end)
      : ASTNodeBase(ctx, begin, end), Text(text) {}
    ~Expr() {}
    virtual void accept(Visitor *visitor, void *data=nullptr) {
      visitor->visit(this, data);
    }
    std::string getText() {return Text;}
    virtual void dump(std::ostream &os) {
      os << "(" << BeginLoc.getLine() << ":" << BeginLoc.getColumn()
         << " Expr)";
    }
  private:
    std::string Text;
  };

  /** @}*/

}



#endif /* AST_V2_H */
