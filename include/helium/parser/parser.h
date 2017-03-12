#ifndef PARSER_H
#define PARSER_H

class Parser {
public:
  Parser(std::string filename);
  ~Parser();

  TranslationUnitDecl *ParseTranslationUnitDecl(XMLNode unit);
  Decl *ParseDecl(XMLNode decl);
  FunctionDecl *ParseFunctionDecl(XMLNode func);
  CompoundStmt *ParseCompoundStmt(XMLNode node);
  Stmt *ParseStmt(XMLNode node);
  IfStmt *ParseIfStmt(XMLNode node);
  IfStmt *ParseElseIfAsIfStmt(XMLNode node);
  SwitchStmt *ParseSwitchStmt(XMLNode node);
  CaseStmt *ParseCaseStmt(XMLNode node);
  DefaultStmt *ParseDefaultStmt(XMLNode node);
  WhileStmt *ParseWhileStmt(XMLNode node);
  Stmt *ParseBlockAsStmt(XMLNode node);
  ForStmt *ParseForStmt(XMLNode node);
  DoStmt *ParseDoStmt(XMLNode node);
  Expr *ParseExpr(XMLNode node);
private:
  void match(XMLNode node, std::string tag) {
    assert(tag == node.name());
  }
  ASTContext *Context;
};

#endif /* PARSER_H */
