#ifndef GENERATOR_H
#define GENERATOR_H




#include "common.h"
#include "parser/ast.h"
#include "parser/ast_node.h"

class CodeGen {
public:
  CodeGen() {}
  ~CodeGen() {}
  // void SetFirstAST(AST *ast) {
  //   m_first_ast = ast;
  // }
  void SetFirstNode(ASTNode *first_node) {
    m_first_node = first_node;
    m_first_ast = m_first_node->GetAST();
  }
  void AddNode(ASTNode *astnode) {
    if (!astnode) return;
    m_data[astnode->GetAST()].insert(astnode);
  }
  // void SetInput(std::map<std::string, Type*> inputs);
  void SetInputs(std::vector<Variable*> inputs) {
    m_inputs = inputs;
  }
  // void SetInputWithoutDecl(std::map<std::string, Type*> inputs);
  std::string GetMain();
  std::string GetSupport();
  std::string GetMakefile();
  void Preprocess();
private:
  std::string getSupportBody();
  void resolveSnippet(AST *ast);
  void resolveSnippet();
  AST *m_first_ast = NULL;
  ASTNode *m_first_node = NULL;
  std::map<AST*, std::set<ASTNode*> > m_data;
  std::set<int> m_snippet_ids;
  // std::map<std::string, Type*> m_inputs;
  // std::map<std::string, Type*> m_inputs_without_decl;
  std::vector<Variable*> m_inputs;
};



#endif /* GENERATOR_H */
