#ifndef POPULATION_H
#define POPULATION_H

#include "ast_node.h"
#include "snippet.h"

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
  void ResolveSnippet();
  std::string GetMain(size_t idx);
  std::string GetSupport(size_t idx);
  std::string GetMakefile();
  std::set<Snippet*> GetSnippets(size_t idx) {
    return m_snippets[m_genes[idx]];
  }
  void Complete();
private:
  void clearGene() {
    for (Gene *g : m_genes) {
      delete g;
    }
    m_genes.clear();
  }
  AST *m_ast;
  std::vector<Gene*> m_genes;
  std::map<Gene*, std::set<Snippet*> > m_snippets;
  std::map<Gene*, Gene> m_cgene_m;
  std::map<Gene*, Gene> m_defuse_gene_m;
  std::map<Gene*, std::string> m_function_m;
};

#endif /* POPULATION_H */
