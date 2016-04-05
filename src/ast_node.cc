#include "ast_node.h"
#include <gtest/gtest.h>
#include "utils.h"



using namespace ast;
static const std::map<ast::NodeKind, ast::ASTNodeKind> nk2ank_m {
  {NK_Function, ANK_Function}
  , {NK_Block, ANK_Block}
  // statements
  , {NK_Stmt, ANK_Stmt}
  , {NK_ExprStmt, ANK_Stmt}
  , {NK_DeclStmt, ANK_Stmt}
  , {NK_Return, ANK_Stmt}
  , {NK_Break, ANK_Stmt}
  , {NK_Continue, ANK_Stmt}
  , {NK_Return, ANK_Stmt}
  // , {NK_EmptyStmt, ANK_Stmt}
  , {NK_Expr, ANK_Expr}
  // condition
  , {NK_If, ANK_If}
  , {NK_Else, ANK_Else}
  , {NK_ElseIf, ANK_ElseIf}
  , {NK_Then, ANK_Then}
  , {NK_Switch, ANK_Switch}
  , {NK_Case, ANK_Case}
  , {NK_Default, ANK_Default}
  // loop
  , {NK_While, ANK_While}
  , {NK_For, ANK_For}
  , {NK_Do, ANK_Do}
};

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

void Gene::Rand(size_t size) {
  m_size = size;
  std::vector<int> flat;
  for (size_t i=0;i<size;i++) {
    int a = utils::rand_int(0, 2);
    if (a==0) { // 1/3
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

size_t AST::leaf_size() {
  int ret =0;
  for (ASTNode *node : m_nodes) {
    if (node->GetChildren().empty()) {
      ret ++;
    }
  }
  return ret;
}



ASTNode* ASTNodeFactory::CreateASTNode(XMLNode xml_node, ASTNode* parent, AST *ast) {
  ast::NodeKind nk = ast::kind(xml_node);
  ASTNode *ret = NULL;
  ast::ASTNodeKind ank;
  if (nk2ank_m.count(nk) == 1) {
    ank = nk2ank_m.at(nk);
  } else {
    ank = ANK_Other;
  }
  switch (ank) {
  case ANK_Function: {
    ret = new Function(xml_node, parent, ast);
    break;
  }
  case ANK_Block: {
    ret = new Block(xml_node, parent, ast);
    break;
  }
  case ANK_Stmt: {
    ret = new Stmt(xml_node, parent, ast);
    break;
  }
  // case ANK_Expr: {
  //   ret = new Expr(xml_node, parent, ast);
  //   break;
  // }
  case ANK_If: {
    ret = new If(xml_node, parent, ast);
    break;
  }
  case ANK_Then: {
    ret = new Then(xml_node, parent, ast);
    break;
  }
  case ANK_Else: {
    ret = new Else(xml_node, parent, ast);
    break;
  }
  case ANK_ElseIf: {
    ret = new ElseIf(xml_node, parent, ast);
    break;
  }
  case ANK_Switch: {
    ret = new Switch(xml_node, parent, ast);
    break;
  }
  case ANK_Case: {
    ret = new Case(xml_node, parent, ast);
    break;
  }
  case ANK_Default: {
    ret = new Default(xml_node, parent, ast);
    break;
  }
  case ANK_While: {
    ret = new While(xml_node, parent, ast);
    break;
  }
  case ANK_For: {
    ret = new For(xml_node, parent, ast);
    break;
  }
  case ANK_Do: {
    ret = new Do(xml_node, parent, ast);
    break;
  }
  case ANK_Other: {
    // ret = new ASTOther(xml_node);
    // TODO for other xml node(like <unit>),
    // we need to smartly get the valid chlidren of that nodes,
    // and have edges from this to those nodes.
    // But currently, just NULL.
    ret = NULL;
    break;
  }
  default: {
    assert(false);
  }
  }
  return ret;
}

std::string AST::GetFunctionName() {
  if (m_nodes.size() == 0) return "";
  ASTNode *func_node = m_nodes[0];
  if (func_node->Kind() != ANK_Function) return "";
  Function *func = dynamic_cast<Function*>(func_node);
  return func->GetName();
}

/**
 * Get leaf nodes, i.e. statements
 */
std::set<ASTNode*> AST::GetLeafNodes() {
  std::set<ASTNode*> ret;
  for (ASTNode* node : m_nodes) {
    if (node->GetChildren().empty()) {
      ret.insert(node);
    }
  }
  return ret;
}


/**
 * Root's lvl is 0. Then 1, 2, 3, ...
 */
int AST::computeLvl(ASTNode* node) {
  int lvl = -1;
  while (node) {
    lvl++;
    node = node->GetParent();
  }
  return lvl;
}
ASTNode *AST::ComputeLCA(std::set<ASTNode*> nodes) {
// std::set<ASTNode*> AST::CompleteGene(std::set<ASTNode*> gene) {
  ASTNode* ret = NULL;
  if (nodes.size() == 0) return NULL;
  if (nodes.size() == 1) return *nodes.begin();
  /**
   * Now, the nodes have 2 or more nodes, so they must converge
   * Compute all convergences, and the top
   * The method is to let all the nodes "grow" at the step(speed)
   * Record each convergence when it appeared in the visit history
   * Keep the unvisited ones, as well as the root.
   * If there're two roots or more, return the root, because that means two lines converges at the root.
   */
  std::set<ASTNode*> worklist = nodes;
  std::set<ASTNode*> visited(worklist.begin(), worklist.end());
  std::set<ASTNode*> convergences;
  while (worklist.size() > 1) {
    std::set<ASTNode*> tmp;
    for (ASTNode* node : worklist) {
      ASTNode *par = node->GetParent();
      if (par == NULL) {
        // node is root. insert it
        assert(node == m_root);
        // if there're 2 or more roots, simply return the root node as LCA
        // this will ensure the finally size of worklist is exactly 1
        if (worklist.count(m_root) > 1) return m_root;
        tmp.insert(node);
      } else if (visited.count(par) == 0) {
        // not seen, insert the parent, but do not keep the node itself
        tmp.insert(par);
        visited.insert(par);
      } else {
        // have seen, record as convergence
        convergences.insert(par);
      }
    }
    worklist = tmp;
  }
  assert(worklist.size() == 1);
  ASTNode *top = *worklist.begin();
  /**
   * There should be convergences.
   * The LCA is the convergence that is closest to the "top"
   */
  ret = *convergences.begin();
  size_t best_d = dist(ret, top);
  for (ASTNode* node : convergences) {
    size_t d = dist(node, top);
    if (d < best_d) {
      best_d = d;
      ret = node;
    }
  }
  return ret;
}

/*******************************
 ** AST 
 *******************************/

/**
 * Compute the distance from low to high.
 * hight must be an ancestor of low.
 */
size_t AST::dist(ASTNode* low, ASTNode* high) {
  size_t ret = 0;
  while (low != high) {
    low = low->GetParent();
    ret++;
  }
  return ret;
}


// ASTNode *AST::ComputeLCA(std::set<ASTNode*> nodes) {
//   std::cout <<"compute LCA"  << "\n";
//   if (nodes.size() == 0) return NULL;
//   std::set<ASTNode*> cands;
//   int best_lvl=computeLvl(*nodes.begin());
//   cands.insert(*nodes.begin());
//   std::cout <<"1"  << "\n";
//   for (ASTNode* node : nodes) {
//     int lvl = computeLvl(node);
//     if (lvl == best_lvl) {
//       cands.insert(node);
//     } else if (lvl < best_lvl) {
//       best_lvl = lvl;
//       cands.clear();
//       cands.insert(node);
//     }
//   }
//   std::cout <<"2"  << "\n";
//   while (cands.size() != 1) {
//     std::set<ASTNode*> newcands;
//     for (ASTNode* node : cands) {
//       if (node->GetParent()) {
//         newcands.insert(node->GetParent());
//       } else {
//         // ROOT
//         return m_root;
//       }
//     }
//     cands = newcands;
//   }
//   std::cout <<"3"  << "\n";
//   return *cands.begin();
// }

std::vector<ASTNode*> AST::getPath(ASTNode *node, ASTNode* lca) {
  std::vector<ASTNode*> ret;
  ASTNode *back = node;
  if (lca==NULL || node == NULL) {
    std::cout <<"NULL!"  << "\n";
  }
  ret.push_back(node);
  while (node != lca) {
    if (node == NULL) {
      std::cout <<"NULL"  << "\n";
      std::cout <<back->GetLabel()  << "\n";
      std::cout <<lca->GetLabel()  << "\n";
      VisualizeN({back, lca}, {});
    }
    node = node->GetParent();
    ret.push_back(node);
  }
  ret.push_back(lca);
  return ret;
}

std::set<ASTNode*> AST::CompleteGene(std::set<ASTNode*> gene) {
  std::set<ASTNode*> ret;
  ASTNode* lca = ComputeLCA(gene);
  // std::string dot = VisualizeN({lca}, gene);
  // utils::visualize_dot_graph(dot);
  for (ASTNode *node : gene) {
    std::vector<ASTNode*> path = getPath(node, lca);
    ret.insert(path.begin(), path.end());
  }
  return ret;
}

std::set<ASTNode*> AST::CompleteGene(Gene *gene) {
  std::set<int> indice = gene->GetIndiceS();
  // back up original nodes
  std::set<ASTNode*> original_nodes = Index2Node(indice);
  // the nodes representing gene growing
  std::set<ASTNode*> nodes = original_nodes;
  std::set<ASTNode*> ret;
  ASTNode *lca = ComputeLCA(nodes);
  for (ASTNode *node : nodes) {
    std::vector<ASTNode*> path = getPath(node, lca);
    nodes.insert(path.begin(), path.end());
  }
  indice = Node2Index(nodes);
  gene->SetIndiceS(indice, gene->size());
  // constructing new set
  for (ASTNode * n : nodes) {
    if (original_nodes.count(n) == 0) {
      ret.insert(n);
    }
  }
  return ret;
}






/**
 * constructing AST from the root node.
 */
ast::AST::AST(ast::XMLNode node) {
  // create root, and the root will create everything else
  m_root = ASTNodeFactory::CreateASTNode(node, NULL, this);
  if (!m_root) return;
  // depth-first-search to traverse(keep) all nodes
  dfs(m_root, m_nodes);
  /**
   * create idx map
   */
  for (size_t i=0, end=m_nodes.size();i<end;i++) {
    m_idx_m[m_nodes[i]] = i;
  }
}

void ast::dfs(ASTNode *node, std::vector<ASTNode*> &visited) {
  visited.push_back(node);
  for (auto it=node->children_begin(),end=node->children_end();it!=end;++it) {
    dfs(*it, visited);
  }
}

void SymbolTable::local_dump() {
  for (auto m : m_map) {
    std::cout <<m.first << ":" << m.second->GetNode()->GetLabel()  << "\n";
  }
}
void SymbolTable::dump() {
  SymbolTable *tbl = this;
  while (tbl) {
    std::cout <<"---- TBL ---"  << "\n";
    tbl->local_dump();
    tbl = tbl->GetParent();
  }
}


AST::~AST() {
  // this will delete everyting tracked by the "children" relation from root
  for (ASTNode *node : m_nodes) {
    delete node;
  }
  for (SymbolTable *tbl : m_sym_tbls) {
    delete tbl;
  }
}

std::string AST::GetCode(std::set<ASTNode*> nodes) {
  if (!m_root) return "";
  if (nodes.empty()) return "";
  std::string ret;
  m_root->GetCode(nodes, ret, false);
  return ret;
}

/**
 * Get all code
 */
std::string AST::GetCode() {
  if (!m_root) return "";
  std::string ret;
  m_root->GetCode({}, ret, true);
  return ret;
}

std::string AST::Visualize(std::string filename) {
  std::set<int> dia_s;
  // std::set<int> fill_s;
  // return VisualizeI(dia_s, fill_s);
  return VisualizeI({}, {}, filename);
}

/**
 * The workhorse of Visualization.
 * Other variants calls it.
 * Will show the graph.
 * @return the dot source as string.
 * 
 * first is GreenYellow, second is Cyan
 * first will overwrite second
 * yellow_s will not completely overwrite cyan_s.
 * If they overlap, the color will be yellow_s, but the node shape will be dotted.
 */
std::string AST::VisualizeI(std::set<int> yellow_s, std::set<int> cyan_s, std::string filename) {
  std::string dot;
  dot+= "digraph {\n";

  for (size_t i=0, size=m_nodes.size();i<size;i++) {
    std::string name = std::to_string(i);
    std::string label = m_nodes[i]->GetLabel();
    // remove all double quotes because it will conflict with label string
    label.erase(std::remove(label.begin(), label.end(), '"'));
    std::string attr = "[";
    attr += "label=\"" + name + ":" + label + "\"";
    /**
     */
    if (yellow_s.count(i) == 1) {
      if (cyan_s.count(i) == 1) {
        attr += ", style=\"filled,dotted\", fillcolor=greenyellow";
      } else {
        attr += ", style=filled, fillcolor=greenyellow";
      }
    } else if (cyan_s.count(i) == 1){
      attr += ", style=filled, fillcolor=cyan";
    }
    attr += "]";
    dot += name + attr + ";\n";
  }
  // edges
  for (ASTNode *node : m_nodes) {
    std::vector<ASTNode*> children = node->Children();
    // std::cout <<children.size()  << "\n";
    for (ASTNode* ch : children) {
      size_t from = m_idx_m[node];
      size_t to = m_idx_m[ch];
      dot += std::to_string(from) + " -> " + std::to_string(to) + ";\n";
    }
  }
  dot += "}";
  utils::visualize_dot_graph(dot, filename);
  return dot;
}
std::string AST::VisualizeN(std::set<ASTNode*> yellow_s, std::set<ASTNode*> cyan_s, std::string filename) {
  std::set<int> yellow_d_s;
  std::set<int> cyan_d_s;
  for (ASTNode *n : yellow_s) {
    yellow_d_s.insert(m_idx_m[n]);
  }
  for (ASTNode *n : cyan_s) {
    cyan_d_s.insert(m_idx_m[n]);
  }
  return VisualizeI(yellow_d_s, cyan_d_s, filename);
}



std::set<int> AST::Node2Index(std::set<ASTNode*> nodes) {
  std::set<int> ret;
  for (ASTNode *node : nodes) {
    ret.insert(m_idx_m[node]);
  }
  return ret;
}
std::vector<int> AST::Node2Index(std::vector<ASTNode*> nodes) {
  std::vector<int> ret;
  for (ASTNode *node : nodes) {
    ret.push_back(m_idx_m[node]);
  }
  return ret;
}
std::set<ASTNode*> AST::Index2Node(std::vector<int> indices) {
  std::set<ASTNode*> ret;
  for (int idx : indices) {
    ret.insert(m_nodes[idx]);
  }
  return ret;
}
std::set<ASTNode*> AST::Index2Node(std::set<int> indices) {
  std::set<ASTNode*> ret;
  for (int idx : indices) {
    ret.insert(m_nodes[idx]);
  }
  return ret;
}

ASTNode *AST::ComputeLCA(std::set<int> indices) {
  std::set<ASTNode*> nodes;
  for (int idx : indices) {
    nodes.insert(m_nodes[idx]);
  }
  return ComputeLCA(nodes);
}

/**
 * FIXME deprecated
 * 1. Get all used variable names in nodes
 * 2. Try to look up their symbol table to locate the nodes that defines them
 */
std::set<ASTNode*> AST::CompleteVarDefUse(std::set<ASTNode*> nodes) {
  std::set<ASTNode*> ret;
  // std::set<ASTNode*> all(nodes.begin(), nodes.end());
  // std::vector<ASTNode*> worklist(nodes.begin(), nodes.end());
  // // for (ASTNode *node : nodes) {
  // while (!worklist.empty()) {
  //   ASTNode *node = worklist.back();
  //   worklist.pop_back();
  //   // wait, I want to build a symbol table for the AST.
  //   // It should not be large, and it should not care about inter-procedure
  //   // std::cout <<" at " << m_idx_m[node] << "\n";
  //   std::set<std::string> ids = node->GetVarIds();
  //   for (std::string id : ids) {
  //     if (id.empty()) continue;
  //     if (is_c_keyword(id)) continue;
  //     // std::cout <<id << " YYY at " << m_idx_m[node] << "\n";
  //     SymbolTable *tbl = node->GetSymTbl();
  //     ASTNode *def = tbl->LookUp(id);
  //     if (def && all.count(def) == 0) {
  //       // this should be in result
  //       ret.insert(def);
  //       all.insert(def);
  //       worklist.push_back(def);
  //     } else {
  //       // std::cout <<": but fail to look up"  << "\n";
  //       // node->GetSymTbl()->dump();
  //     }
  //   }
  // }
  // ret.insert(nodes.begin(), nodes.end());
  return ret;
}

Gene AST::CompleteVarDefUse(Gene gene) {
  // std::set<int> indice = gene.GetIndiceS();
  // std::set<ASTNode*> nodes = Index2Node(indice);
  // nodes = CompleteVarDefUse(nodes);
  // indice = Node2Index(nodes);
  // gene.SetIndiceS(indice, gene.size());
  return gene;
}

