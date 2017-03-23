#include "helium/parser/ast_v2.h"
#include <iostream>

#include "helium/parser/benchmark_manager.h"

using namespace v2;

void TranslationUnitDecl::dump() {
  std::cout << "(unit ";
  for (Decl* decl : decls) {
    if (decl) decl->dump();
  }
  std::cout << ")" << "\n";
}
void FunctionDecl::dump() {
  std::cout << "(function ";
  std::cout << Ctx->getBenchmarkManager()->getIdByNode(this) << ":" << name << " ";
  if (body) {
    body->dump();
  }
  std::cout << ")";
}



void ASTContext::computeLevels() {
  
}
void ASTContext::populateNodes() {
}
