#include "population.h"

void Population::RandGenes(int num) {
  if (!m_ast) return;
  clearGene();
  for (int i=0;i<num;i++) {
    Gene *g = new Gene();
    g->Rand(m_ast->size());
    m_genes.push_back(g);
  }
}

std::string Population::GetCode(size_t idx) {
  if (idx >= m_genes.size()) return "";
  // // Gene *g = m_genes[idx];
  // Gene cg = m_ast->CompleteGene(*m_genes[idx]);

  Gene *g = m_genes[idx];
  Gene cg = m_ast->CompleteGene(*g);
  m_ast->VisualizeI(cg.GetIndiceS(), {}, std::to_string(idx)+"complete");
  Gene defuseg = m_ast->CompleteVarDefUse(cg);

  std::set<ASTNode*> nodes = m_ast->Index2Node(defuseg.GetIndice());
  return m_ast->GetCode(nodes);

}

void Population::Visualize(size_t idx) {
  if (idx >= m_genes.size()) return;
  m_ast->VisualizeI(m_genes[idx]->GetIndiceS(), {}, std::to_string(idx));
  Gene *g = m_genes[idx];
  Gene cg = m_ast->CompleteGene(*g);
  m_ast->VisualizeI(cg.GetIndiceS(), {}, std::to_string(idx)+"complete");
  Gene defuseg = m_ast->CompleteVarDefUse(cg);
  m_ast->VisualizeI(defuseg.GetIndiceS(), {}, std::to_string(idx) + "defuse");
}
