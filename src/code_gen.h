#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "common.h"
#include "parser/ast.h"
#include "parser/ast_node.h"

class CodeGen {
public:
  CodeGen() {}
  ~CodeGen() {}
  void SetFirstAST(AST *ast) {
    m_first_ast = ast;
  }
  void AddNode(ASTNode *astnode) {
    if (!astnode) return;
    m_data[astnode->GetAST()].insert(astnode);
  }
  void SetInput(std::map<std::string, Type*> inputs) {m_inputs = inputs;}
  void Gen() {}
  std::string GetMain();
  std::string GetSupport();
  std::string GetMakefile();
private:
  std::string getSupportBody();
  void resolveSnippet(AST *ast);
  void resolveSnippet();
  AST *m_first_ast;
  std::map<AST*, std::set<ASTNode*> > m_data;
  std::set<int> m_snippet_ids;
  std::map<std::string, Type*> m_inputs;
};



#endif /* CODE_GEN_H */
