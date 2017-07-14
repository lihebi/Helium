#ifndef PARSER_H
#define PARSER_H

#include "helium/parser/AST.h"
#include <string>
#include "helium/utils/XMLNode.h"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;

/**
 * \defgroup parser
 * Parser converts source code to AST. When consuming srcml output, we convert that output to AST.
 */


class Parser {
public:
  Parser() {}
  virtual ~Parser() {}
  virtual ASTContext* parse(fs::path file) = 0;
};

class ClangParser : public Parser {
public:
  ClangParser() {}
  virtual ~ClangParser() {}

  virtual ASTContext *parse(fs::path file);
};

/**
 * \ingroup parser
 * \brief Parser for SrcML.
 *
 * This is the only place related to SrcML (hopefully)
 */
class SrcMLParser : public Parser {
public:
  SrcMLParser() {}
  virtual ~SrcMLParser() {}

  virtual ASTContext *parse(fs::path file);

  TranslationUnitDecl *ParseTranslationUnitDecl(ASTContext *ctx, XMLNode unit);
  DeclStmt *ParseDeclStmt(ASTContext *ctx, XMLNode decl);
  FunctionDecl *ParseFunctionDecl(ASTContext *ctx, XMLNode func);
  CompoundStmt *ParseCompoundStmt(ASTContext *ctx, XMLNode node);
  Stmt *ParseStmt(ASTContext *ctx, XMLNode node);
  ReturnStmt *ParseReturnStmt(ASTContext *ctx, XMLNode node);
  IfStmt *ParseIfStmt(ASTContext *ctx, XMLNode node);
  IfStmt *ParseElseIfAsIfStmt(ASTContext *ctx, XMLNode node);
  SwitchStmt *ParseSwitchStmt(ASTContext *ctx, XMLNode node);
  CaseStmt *ParseCaseStmt(ASTContext *ctx, XMLNode node);
  DefaultStmt *ParseDefaultStmt(ASTContext *ctx, XMLNode node);
  WhileStmt *ParseWhileStmt(ASTContext *ctx, XMLNode node);
  Stmt *ParseBlockAsStmt(ASTContext *ctx, XMLNode node);
  ForStmt *ParseForStmt(ASTContext *ctx, XMLNode node);
  DoStmt *ParseDoStmt(ASTContext *ctx, XMLNode node);
  Expr *ParseExpr(ASTContext *ctx, XMLNode node);
  Expr *ParseExprWithoutSemicolon(ASTContext *ctx, XMLNode node);
  Stmt *ParseExprStmt(ASTContext *ctx, XMLNode node);
private:
  void match(XMLNode node, std::string tag);
};



#endif /* PARSER_H */
