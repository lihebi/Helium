#include "helium/parser/Parser.h"

ASTContext *ClangParser::parse(fs::path file) {
  ASTContext *ctx = new ASTContext(file.string());
  TranslationUnitDecl *unit = nullptr;
  // TODO
  // unit = ParseTranslationUnitDecl(root);
  ctx->setTranslationUnitDecl(unit);
  return ctx;
}
