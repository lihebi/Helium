#ifndef POPULATION_H
#define POPULATION_H

#include "ast_node.h"

using namespace ast;

class Population {
public:
  Population() {}
  ~Population() {
    clearGene();
  }
  void SetAST(AST *ast) {
    m_ast = ast;
  }
  void RandGenes(int num);
  Gene* GetGene(size_t idx) {
    if (idx >= m_genes.size()) return NULL;
    return m_genes[idx];
  }
  std::string GetCode(size_t idx);
  void Visualize(size_t idx);
  size_t size() {
    return m_genes.size();
  }
private:
  void clearGene() {
    for (Gene *g : m_genes) {
      delete g;
    }
    m_genes.clear();
  }
  AST *m_ast;
  std::vector<Gene*> m_genes;
};

#endif /* POPULATION_H */
