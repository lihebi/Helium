#include "ast_node.h"
#include <gtest/gtest.h>
#include "utils.h"
#include "options.h"

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

/********************************
 * AST
 *******************************/

AST* ASTFactory::CreateASTFromDoc(ast::XMLDoc *doc) {
  NodeList func_nodes = find_nodes(*doc, NK_Function);
  if (func_nodes.empty()) return NULL;
  Node func_node = func_nodes[0];
  return new AST(func_node);
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

/**
 * Based on node, get previous LEAF node, in the same procedure.
 * Return NULL if the node is the first node.
 * Assertion failure if node is not in this AST
 */
ASTNode *AST::GetPreviousLeafNode(ASTNode *node) {
  print_trace("AST::GetPreviousLeafNode()");
  assert(Contains(node));
  int idx = GetIndexByNode(node);
  for (int i=idx-1;i>=0;i--) {
    if (m_nodes[i]->GetChildren().empty()) {
      return m_nodes[i];
    }
  }
  return NULL;
}

ASTNode *AST::GetPreviousLeafNodeInSlice(ASTNode *node) {
  assert(node);
  if (m_is_slice_active) {
    // if slice is active, 
    ASTNode *ret = node;
    while (true) {
      ret = GetPreviousLeafNode(ret);
      if (!ret) return NULL;
      if (m_slice.count(ret) == 1) return ret;
    }
  } else {
    // if slice is not active, behavior exactly as GetPreviousLeafNode
    return GetPreviousLeafNode(node);
  }
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

std::set<ASTNode*> AST::CompleteGeneToRoot(std::set<ASTNode*> gene) {
  ASTNode *lca = m_root;
  std::set<ASTNode*> ret;
  for (ASTNode *node :gene) {
    std::vector<ASTNode*> path = getPath(node, lca);
    ret.insert(path.begin(), path.end());
  }
  return ret;
}

/**
 * If the nodes contains continue, break, add the parent
 * - for, while, do
 * - switch
 */
std::set<ASTNode*> patch_syntax(std::set<ASTNode*> nodes) {
  std::set<ASTNode*> ret (nodes.begin(), nodes.end());
  for (ASTNode *n : nodes) {
    if (n->Kind() == ANK_Stmt) {
      if (kind(n->GetXMLNode()) == NK_Break
          || kind(n->GetXMLNode()) == NK_Continue
          ) {
        ASTNode *p = n->GetParent();
        while (p && p->Kind() != ANK_For
               && p->Kind() != ANK_While
               && p->Kind() != ANK_Do
               && p->Kind() != ANK_Switch
               ) {
          p = p->GetParent();
        }
        assert(p);
        ret.insert(p);
      }
    }
  }
  return ret;
}

std::set<ASTNode*> AST::CompleteGene(std::set<ASTNode*> gene) {
  print_trace("AST::CompleteGene(std::set<ASTNode*> gene)");
  std::set<ASTNode*> ret;
  // FIXME before compute lca, need to patch: continue, break
  gene = patch_syntax(gene);
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
  print_trace("AST::CompleteGene(Gene *gene)");
  assert(gene);
  std::set<int> indice = gene->GetIndiceS();
  // back up original nodes
  std::set<ASTNode*> original_nodes = Index2Node(indice);
  // the nodes representing gene growing
  std::set<ASTNode*> nodes = original_nodes;
  std::set<ASTNode*> ret;
  // FIXME before compute lca, need to patch: continue, break
  nodes = patch_syntax(nodes);
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
 * FIXME This should be the only one constructor used?
 */
ast::AST::AST(ast::XMLNode node) {
  print_trace("AST::AST(ast::XMLNode node)");
  // create root, and the root will create everything else
  m_root = ASTNodeFactory::CreateASTNode(node, NULL, this);
  m_filename = ast::unit_get_filename(node);
  if (!m_root) return;
  // depth-first-search to traverse(keep) all nodes
  dfs(m_root, m_nodes);
  /**
   * create idx map
   */
  for (size_t i=0, end=m_nodes.size();i<end;i++) {
    m_idx_m[m_nodes[i]] = i;
  }
  // create xmlnode mapping
  for (ASTNode *n : m_nodes) {
    m_xmlnode_m[n->GetXMLNode()] = n;
  }
}

void AST::SetSlice() {
  // when calling this method, make sure it is valid
  assert(SimpleSlice::Instance()->IsValid());
  std::set<int> linums = SimpleSlice::Instance()->GetLineNumbers(m_filename);
  // for (int l : linums) {
  //   std::cout << l  << " ";
  // }
  // std::cout  << "\n";
  for (ASTNode* node : m_nodes) {
    // TODO for each node model, I need to judge whether it is in slice?
    // e.g. a for loop, the linum should be its for condition.
    // the body of the for loop should not contribute to its (end) linum?
    int begin = node->GetBeginLinum();
    int end = node->GetEndLinum();
    // if between begin and end, there exists any line in the slice, we count it as in the slice
    // This code is ugly
    for (int linum : linums) {
      if (linum >= begin && linum <= end) {
        m_slice.insert(node);
        break;
      }
    }
  }
  m_is_slice_active = true;
}
void AST::ClearSlice() {
  m_slice.clear();
  m_is_slice_active = false;
}

ASTNode* AST::GetCallSite(std::string func_name) {
  assert(m_root);
  XMLNode callsite = ast::find_callsite(m_root->GetXMLNode(), func_name);
  return GetEnclosingNodeByXMLNode(callsite);
}

std::set<ASTNode*> AST::GetCallSites(std::string func_name) {
  assert(m_root);
  // DEBUG
  // XMLNode node = m_root->GetXMLNode();
  // // node.print(std::cout);
  // std::cout << get_text(node) << "\n";
  // std::cout << "finding : " << func_name  << "\n";
  XMLNodeList callsites = ast::find_callsites(m_root->GetXMLNode(), func_name);
  std::set<ASTNode*> ret;
  for (XMLNode node : callsites) {
    ASTNode *n = GetEnclosingNodeByXMLNode(node);
    if (n) {
      ret.insert(n);
    }
  }
  return ret;
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
  // free the symbol tables
  for (SymbolTable *tbl : m_sym_tbls) {
    delete tbl;
  }
}


/**
 * Get LCA, and get node from there.
 * If the nodes are not completed, the result code is not likely to be able to build.
 */
std::string AST::GetCode(std::set<ASTNode*> nodes) {
  if (!m_root) return "";
  if (nodes.empty()) return "";
  std::string ret;
  // add location to AST code output
  ret += "/* " + GetFilename() + ":" + std::to_string(GetLineNumber()) + " */\n";
  // m_root->GetCode(nodes, ret, false);
  ASTNode *lca = ComputeLCA(nodes);
  assert(lca);
  /**
   * Here is a trick: if the lca is not included, (or it is a function node)
   * We need to use its children nodes to get code
   * Otherwise will get nothing
   */
  if (nodes.count(lca) ==1) {
    lca->GetCode(nodes, ret, false);
  } else {
    for (ASTNode * node : lca->Children()) {
      node->GetCode(nodes, ret, false);
    }
  }
  // VisualizeN(nodes, {});
  return ret;
}

/**
 * Hehe, duplicate!
 */
std::string AST::GetFilename() {
  assert(m_root);
  std::string ret;
  XMLNode root = m_root->GetXMLNode().select_node("/unit").node();
  // std::cout << root.name()  << "\n";
  // std::cout << m_root->GetXMLNode().name()  << "\n";
  ret = root.attribute("filename").value();
  // std::cout << ret  << "\n";
  return ret;
}
int AST::GetLineNumber() {
  return ast::get_node_line(m_root->GetXMLNode());
}

/**
 * Get node by line number query
 * Possibly return NULL, because the srcml inserts position information weirdly.
 */
ASTNode* AST::GetNodeByLinum(int linum) {
  // traverse the nodes, get the ones that linum \in [begin, end]
  // retain the one with minimum end-begin
  // this is the minimum span of an ASTNode
  // I don't believe the result can be larger than 1000, if YES, xixishuiba
  // std::cout << "required linum: " << linum  << "\n";
  int min_span = 1000;
  ASTNode *ret = NULL;
  std::vector<ASTNode*> nodes;
  for (ASTNode *node : m_nodes) {
    int begin = node->GetBeginLinum();
    int end = node->GetEndLinum();
    // std::cout << "begin: " << begin  << "\n";
    // std::cout << "end: " << end  << "\n";
    if (begin <= linum && end >= linum) {
      if (min_span > end-begin) {
        ret = node;
      }
    }
  }
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
 * TODO use dot.h utilities
 */
std::string AST::VisualizeI(std::set<int> yellow_s, std::set<int> cyan_s, std::string filename) {
  std::string dot;
  dot+= "digraph {\n";

  for (size_t i=0, size=m_nodes.size();i<size;i++) {
    std::string name = std::to_string(i);
    std::string label = m_nodes[i]->GetLabel();
    // remove all double quotes because it will conflict with label string
    label.erase(std::remove(label.begin(), label.end(), '"'), label.end());
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
  std::string dot_file = utils::visualize_dot_graph(dot, filename);
  std::cout << "written to " << dot_file  << "\n";
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

std::string AST::VisualizeSlice(std::string filename) {
  return VisualizeN(m_slice, {});
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

std::set<ASTNode*> AST::RemoveRoot(std::set<ASTNode*> nodes) {
  if (nodes.count(m_root) == 1) {
    nodes.erase(m_root);
  }
  return nodes;
}
