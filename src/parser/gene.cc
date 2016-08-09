#include "ast.h"
#include "utils/utils.h"
#include "ast_node.h"

/*******************************
 ** Gene 
 *******************************/
void Gene::SetFlat(std::vector<int> flat) {
  m_size = flat.size();
  m_indice.clear();
  m_indice_s.clear();
  m_flat.clear();
  m_flat = flat;
  for (size_t i=0;i<flat.size();i++) {
    if (flat[i] == 1) {
      m_indice.push_back(i);
      m_indice_s.insert(i);
    }
  }
}

void Gene::SetFlat(std::string s) {
  std::vector<int> flat;
  for (char c : s) {
    if (c == '1') flat.push_back(1);
    else flat.push_back(0);
  }
  SetFlat(flat);
}
void Gene::SetIndice(std::vector<int> indice, size_t size) {
  m_size = size;
  m_indice.clear();
  m_indice_s.clear();
  m_flat.clear();
  m_indice = indice;
  m_indice_s.insert(indice.begin(), indice.end());
  for (size_t i=0;i<size;i++) {
    if (m_indice_s.count(i) == 1) m_flat.push_back(1);
    else m_flat.push_back(0);
  }
}
void Gene::SetIndiceS(std::set<int> indice_s, size_t size) {
  m_size = size;
  m_indice.clear();
  m_indice_s.clear();
  m_flat.clear();
  m_indice_s = indice_s;
  m_indice = std::vector<int>(indice_s.begin(), indice_s.end());
  std::sort(m_indice.begin(), m_indice.end());
  for (size_t i=0;i<size;i++) {
    if (m_indice_s.count(i) == 1) m_flat.push_back(1);
    else m_flat.push_back(0);
  }
}

void Gene::AddNode(ASTNode *node) {
  int idx = m_ast->GetIndexByNode(node);
  std::set<int> tmp_indice_s = m_indice_s;
  tmp_indice_s.insert(idx);
  SetIndiceS(tmp_indice_s, tmp_indice_s.size());
}
Gene::Gene(AST *ast) : m_ast(ast) {
  m_size = m_ast->size();
}

void Gene::Rand() {
  std::vector<int> flat;
  for (size_t i=0;i<m_size;i++) {
    int a = utils::rand_int(0, 1);
    if (a==0) { // 1/2
      flat.push_back(1);
    } else {
      flat.push_back(0);
    }
  }
  SetFlat(flat);
}

/**
 * Assign leaf node to randomly 0 or 1
 */
void Gene::LeafRand() {
  assert(m_ast);
  std::set<ASTNode*> leafs = m_ast->GetLeafNodes();
  std::set<int> indice = m_ast->Node2Index(leafs);
  // randomly remove from indice
  for (auto it=indice.begin(), end=indice.end();it!=end;) {
    int a = utils::rand_int(0, 2);
    if (a==0) { //1/3
      // retain it
      it++;
    } else {
      it = indice.erase(it);
    }
  }
  SetIndiceS(indice, m_ast->size());
}

void Gene::dump() {
  for (int g : m_flat) {
    std::cout << g;
  }
  std::cout  << "\n";
}

std::set<ASTNode*> Gene::ToASTNodeSet() {
  return m_ast->Index2Node(m_indice_s);
}

/**
 * how many leaf nodes selected by the gene
 */
size_t Gene::leaf_size() {
  std::set<ASTNode*> nodes = m_ast->Index2Node(m_indice_s);
  int ret=0;
  for (ASTNode* node : nodes) {
    if (node->GetChildren().empty()) ret++;
  }
  return ret;
}
