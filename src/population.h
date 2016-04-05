#ifndef POPULATION_H
#define POPULATION_H

#include "ast_node.h"
#include "snippet.h"

using namespace ast;

class Individual {
public:
  Individual(AST *ast) : m_ast(ast) {
    assert(ast);
  }
  ~Individual() {
    if (m_gene) delete m_gene;
  }
  void RandGene();
  Gene* GetGene();
  void Visualize();
  std::string GetMain();
  std::string GetSupport();
  std::string GetMakefile();
  std::string GetCode();
  void Solve();
  void ResolveSnippet();
  std::set<Snippet*> GetSnippets() {
    return m_snippets;
  }
private:
  Gene *m_gene = NULL;
  AST *m_ast = NULL;
  // from the decl node to the variable needed to be declared
  std::map<ASTNode*, std::set<std::string> > m_decl_input_m;
  // do not need input
  std::map<ASTNode*, std::set<std::string> > m_decl_m;
  std::set<Snippet*> m_snippets;
};

class Population {
public:
  Population(AST *ast) : m_ast(ast) {
    assert(ast);
  }
  ~Population() {
    for (Individual *ind : m_individuals) {
      delete ind;
    }
  }
  void CreateRandomIndividuals(int num);
  std::vector<Individual*> GetIndividuals() {
    return m_individuals;
  }
  
  size_t size() {
    return m_individuals.size();
  }
  void ResolveSnippet();
  void Complete();
  void Solve();
private:
  AST *m_ast;
  // std::vector<Gene*> m_genes;
  // std::map<Gene*, std::set<Snippet*> > m_snippets;
  // std::map<Gene*, Gene> m_cgene_m;
  // std::map<Gene*, Gene> m_defuse_gene_m;
  // std::map<Gene*, std::string> m_function_m;
  std::vector<Individual*> m_individuals;
};

#endif /* POPULATION_H */
