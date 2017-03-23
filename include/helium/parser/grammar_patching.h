#ifndef GRAMMAR_PATCHING_H
#define GRAMMAR_PATCHING_H

#include <vector>
#include "helium/parser/ast_v2.h"
#include "helium/utils/common.h"

class GrammarPatcher {
public:
  GrammarPatcher() {}
  ~GrammarPatcher() {}
  /**
   * according to current selection, decide what else needs to be selected
   */
  void grammarPatch();
  /**
   * generate partial program based on selection
   */
  std::string generatePartialProgram();
  void addSelect(v2::ASTNodeBase *node) {
    if (node) {
      Select.insert(node);
    }
  }
private:
  std::set<v2::ASTNodeBase*> Select;
};

#endif /* GRAMMAR_PATCHING_H */
