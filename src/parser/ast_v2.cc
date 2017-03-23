#include "helium/parser/ast_v2.h"
#include <iostream>

namespace v2 {
  void TranslationUnitDecl::dump() {
    std::cout << "(unit ";
    for (Decl* decl : decls) {
      if (decl) decl->dump();
    }
    std::cout << ")" << "\n";
  }
  void FunctionDecl::dump() {
    std::cout << "(function ";
    std::cout << name << " ";
    if (body) {
      body->dump();
    }
    std::cout << ")";
  }
}

