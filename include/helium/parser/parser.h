#ifndef PARSER_H
#define PARSER_H

#include "helium/parser/ast_v2.h"
#include <string>
#include "helium/parser/xmlnode.h"

class Parser {
public:
  Parser(std::string filename);
  ~Parser();

  v2::TranslationUnitDecl *ParseTranslationUnitDecl(XMLNode unit);
  v2::DeclStmt *ParseDeclStmt(XMLNode decl);
  v2::FunctionDecl *ParseFunctionDecl(XMLNode func);
  v2::CompoundStmt *ParseCompoundStmt(XMLNode node);
  v2::Stmt *ParseStmt(XMLNode node);
  v2::IfStmt *ParseIfStmt(XMLNode node);
  v2::IfStmt *ParseElseIfAsIfStmt(XMLNode node);
  v2::SwitchStmt *ParseSwitchStmt(XMLNode node);
  v2::CaseStmt *ParseCaseStmt(XMLNode node);
  v2::DefaultStmt *ParseDefaultStmt(XMLNode node);
  v2::WhileStmt *ParseWhileStmt(XMLNode node);
  v2::Stmt *ParseBlockAsStmt(XMLNode node);
  v2::ForStmt *ParseForStmt(XMLNode node);
  v2::DoStmt *ParseDoStmt(XMLNode node);
  v2::Expr *ParseExpr(XMLNode node);

  v2::TranslationUnitDecl *getTranslationUnit() {return unit;}
private:
  void match(XMLNode node, std::string tag);
  v2::ASTContext *Context = nullptr;
  v2::TranslationUnitDecl *unit = nullptr;
};

#endif /* PARSER_H */
