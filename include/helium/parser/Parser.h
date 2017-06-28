#ifndef PARSER_H
#define PARSER_H

#include "helium/parser/AST.h"
#include <string>
#include "helium/utils/XMLNode.h"

/**
 * \defgroup parser
 * Parser converts source code to AST. When consuming srcml output, we convert that output to AST.
 */

/**
 * \ingroup parser
 * \brief Parser for SrcML.
 *
 * This is the only place related to SrcML (hopefully)
 */
class Parser {
public:
  Parser(std::string filename);
  ~Parser();

  TranslationUnitDecl *ParseTranslationUnitDecl(XMLNode unit);
  DeclStmt *ParseDeclStmt(XMLNode decl);
  FunctionDecl *ParseFunctionDecl(XMLNode func);
  CompoundStmt *ParseCompoundStmt(XMLNode node);
  Stmt *ParseStmt(XMLNode node);
  ReturnStmt *ParseReturnStmt(XMLNode node);
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
  Expr *ParseExprWithoutSemicolon(XMLNode node);
  Stmt *ParseExprStmt(XMLNode node);

  ASTContext *getASTContext() {return Ctx;}
private:
  void match(XMLNode node, std::string tag);
  ASTContext *Ctx = nullptr;
};

#endif /* PARSER_H */
