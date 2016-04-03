#include "ast_node.h"
#include <gtest/gtest.h>
#include "utils.h"

#define DEBUG_AST_NODE_TRACE
#undef DEBUG_AST_NODE_TRACE



using namespace ast;
static const std::map<ast::NodeKind, ast::ASTNodeKind> nk2ank_m {
  {NK_Function, ANK_Function}
  , {NK_Block, ANK_Block}
  , {NK_Stmt, ANK_Stmt}
  , {NK_ExprStmt, ANK_Stmt}
  , {NK_DeclStmt, ANK_Stmt}
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

void Gene::dump() {
  for (int g : m_flat) {
    std::cout << g;
  }
  std::cout  << "\n";
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
  case ANK_Expr: {
    ret = new Expr(xml_node, parent, ast);
    break;
  }
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
    std::cout <<m.first << ":" << m.second->GetLabel()  << "\n";
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

Gene AST::CompleteGene(Gene gene) {
  std::set<int> indice = gene.GetIndiceS();
  std::set<ASTNode*> nodes = Index2Node(indice);
  nodes = CompleteGene(nodes);
  indice = Node2Index(nodes);
  gene.SetIndiceS(indice, gene.size());
  return gene;
}

ASTNode *AST::ComputeLCA(std::set<int> indices) {
  std::set<ASTNode*> nodes;
  for (int idx : indices) {
    nodes.insert(m_nodes[idx]);
  }
  return ComputeLCA(nodes);
}

std::set<ASTNode*> AST::CompleteVarDefUse(std::set<ASTNode*> nodes) {
  std::set<ASTNode*> ret;
  std::set<ASTNode*> all(nodes.begin(), nodes.end());
  std::vector<ASTNode*> worklist(nodes.begin(), nodes.end());
  // for (ASTNode *node : nodes) {
  while (!worklist.empty()) {
    ASTNode *node = worklist.back();
    worklist.pop_back();
    // wait, I want to build a symbol table for the AST.
    // It should not be large, and it should not care about inter-procedure
    // std::cout <<" at " << m_idx_m[node] << "\n";
    std::set<std::string> ids = node->GetVarIds();
    for (std::string id : ids) {
      if (id.empty()) continue;
      if (is_c_keyword(id)) continue;
      // std::cout <<id << " YYY at " << m_idx_m[node] << "\n";
      SymbolTable *tbl = node->GetSymTbl();
      ASTNode *def = tbl->LookUp(id);
      if (def && all.count(def) == 0) {
        // this should be in result
        ret.insert(def);
        all.insert(def);
        worklist.push_back(def);
      } else {
        // std::cout <<": but fail to look up"  << "\n";
        // node->GetSymTbl()->dump();
      }
    }
  }
  ret.insert(nodes.begin(), nodes.end());
  return ret;
}

Gene AST::CompleteVarDefUse(Gene gene) {
  std::set<int> indice = gene.GetIndiceS();
  std::set<ASTNode*> nodes = Index2Node(indice);
  nodes = CompleteVarDefUse(nodes);
  indice = Node2Index(nodes);
  gene.SetIndiceS(indice, gene.size());
  return gene;
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


/*******************************
 ** AST Node 
 *******************************/


/*******************************
 ** General
 *******************************/

Expr::Expr(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  assert(false);
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_parent->GetSymTbl();
  }
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- Expr" << "\n";
  #endif
}
void Expr::GetCode(std::set<ASTNode*> nodes,
                   std::string &ret, bool all, bool add_brace) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
}

Stmt::Stmt(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_parent->GetSymTbl();
  }
  /**
   * push the symbol table
   */
  if (kind(xmlnode) == NK_DeclStmt) {
    XMLNodeList decls = ast::decl_stmt_get_decls(xmlnode);
    for (XMLNode decl : decls) {
      std::string declname = decl_get_name(decl);
      m_sym_tbl->AddSymbol(declname, this);
    }
  }
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- Stmt" << "\n";
  #endif
}
void Stmt::GetCode(std::set<ASTNode*> nodes,
                   std::string &ret, bool all, bool add_brace) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += get_text(m_xmlnode);
  }
  // ret += "\n";
}


/**
 * This is NOT a ASTNode! Just itself.
 */
Decl::Decl(XMLNode xmlnode) {
  m_xmlnode = xmlnode;
  // m_type
  m_type = decl_get_type(xmlnode);
  // m_name
  m_name = decl_get_name(xmlnode);
}

/******************************
 ** Function
 *******************************/

Function::Function(XMLNode xmlnode, ASTNode *parent, AST *ast) {
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymTbl());
  }
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- Function" << "\n";
  #endif
  // m_ret_ty = xmlnode.child("type").child_value("name"); // function_get_return_type(xmlnode);
  m_ret_ty = function_get_return_type(xmlnode);
  m_name = xmlnode.child_value("name"); // function_get_name(xmlnode);
  XMLNode blk_n = xmlnode.child("block"); // function_get_block(xmlnode);
  m_blk = new Block(blk_n, this, ast);
  XMLNodeList nodes = function_get_param_decls(xmlnode);
  for (XMLNode node : nodes) {
    // needs to be delete'd
    m_params.push_back(new Decl(node));
  }
  // constructnig children
  m_children.push_back(m_blk);
  /**
   * Push the symbol table
   */
  for (Decl *decl : m_params) {
    m_sym_tbl->AddSymbol(decl->GetName(), this);
  }
}

Function::~Function() {
  for (Decl *decl : m_params) {
    delete decl;
  }
}

void Function::GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all, bool add_brace) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  // if (!nodes.empty() && nodes.count(this) == 0) return;
  // temporarily disable this, i.e. no function decl, only body
  // so that it is valid to be put in a main function
  if (selected) {
    ret += m_ret_ty + " " + m_name + "(";
    if (!m_params.empty()) {
      for (Decl *param : m_params) {
        ret += param->GetType();
        ret += " ";
        ret += param->GetName();
        ret += ",";
      }
      ret.pop_back(); // remove last ","
    }
    ret += ")";
  }
  m_blk->GetCode(nodes, ret, all, selected);
}

std::string Function::GetLabel() {
  // return m_name;
  std::string ret;
  ret += m_name + "(";
  if (!m_params.empty()) {
    for (Decl *param : m_params) {
      ret += param->GetType();
      ret += " ";
      ret += param->GetName();
      ret += ",";
    }
    ret.pop_back(); // remove last ","
  }
  ret += ")";
  return ret;
}


Block::Block(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymTbl());
  }
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- Block" << "\n";
  #endif
  // constructing children
  XMLNodeList nodes = block_get_nodes(xmlnode);
  for (XMLNode node : nodes) {
    ASTNode *anode = ASTNodeFactory::CreateASTNode(node, this, ast);
    if (anode) {
      // anode->SetParent(this);
      m_children.push_back(anode);
    }
  }
}

void Block::GetCode(std::set<ASTNode*> nodes,
                    std::string &ret, bool all, bool add_brace) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  // if (!nodes.empty() && nodes.count(this) == 0) return;
  if (selected || add_brace) {
    ret += "{\n";
  }
  for (ASTNode *n : m_children) {
    n->GetCode(nodes, ret, false);
  }
  if (selected || add_brace) {
    ret += "\n}";
  }
}

/*******************************
 ** Condition
 *******************************/

If::If(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymTbl());
  }
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- IF" << "\n";
  #endif
  m_cond = if_get_condition_expr(xmlnode);
  XMLNode then_node = if_get_then(xmlnode);
  XMLNode else_node = if_get_else(xmlnode);
  XMLNodeList nodes = if_get_elseifs(xmlnode);
  if (then_node) {
    m_then = new Then(then_node, this, ast);
    m_children.push_back(m_then);
  }
  for (XMLNode node : nodes) {
    ElseIf *anode = new ElseIf(node, this, ast);
    m_elseifs.push_back(anode);
  }
  m_children.insert(m_children.end(), m_elseifs.begin(), m_elseifs.end());
  if (else_node) {
    m_else = new Else(else_node, this, ast);
    m_children.push_back(m_else);
  }
}

void If::GetCode(std::set<ASTNode*> nodes,
                 std::string &ret, bool all, bool add_brace) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "if (";
    ret += get_text(m_cond);
    ret += ")";
  }
  m_then->GetCode(nodes, ret, all, selected);
  for (ElseIf *ei : m_elseifs) {
    ei->GetCode(nodes, ret, all);
  }
  if (m_else) {
    m_else->GetCode(nodes, ret, all);
  }
}

std::string If::GetLabel() {
  // with condition
  std::string ret;
  ret += "if (" + get_text(m_cond) + ")";
  return ret;
}


Then::Then(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  m_parent = parent;
  m_xmlnode = xmlnode;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymTbl());
  }
  XMLNode node = then_get_block(xmlnode);
  m_blk = new Block(node, this, ast);
  m_children.push_back(m_blk);
}

void Then::GetCode(std::set<ASTNode*> nodes,
                   std::string &ret, bool all, bool add_brace) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
  // if (!nodes.empty() && nodes.count(this) == 0) {
  //   ret += "{}";
  //   return;
  // }
  m_blk->GetCode(nodes, ret, add_brace);
}

Else::Else(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  m_parent = parent;
  m_xmlnode = xmlnode;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymTbl());
  }
  XMLNode node = else_get_block(xmlnode);
  m_blk = new Block(node, this, ast);
  m_children.push_back(m_blk);
}
void Else::GetCode(std::set<ASTNode*> nodes,
                   std::string &ret, bool all, bool add_brace) {
  // if (!nodes.empty() && nodes.count(this) == 0) return;
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "else";
  }
  m_blk->GetCode(nodes, ret, all, selected);
}

ElseIf::ElseIf(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  m_parent = parent;
  m_xmlnode = xmlnode;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymTbl());
  }
  XMLNode node = elseif_get_block(xmlnode);
  m_cond = elseif_get_condition_expr(xmlnode);
  m_blk = new Block(node, this, ast);
  m_children.push_back(m_blk);
}

void ElseIf::GetCode(std::set<ASTNode*> nodes,
                     std::string &ret, bool all, bool add_brace) {
  // if (!nodes.empty() && nodes.count(this) == 0) return;
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "else if(" + get_text(m_cond) + ")";
  }
  m_blk->GetCode(nodes, ret, all, selected);
}

std::string ElseIf::GetLabel() {
  std::string ret;
  ret += "elseif (" + get_text(m_cond) + ")";
  return ret;
}


/**
 * Switch is not used currently.
 */
Switch::Switch(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymTbl());
  }
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- SWITCH" << "\n";
  #endif
  XMLNodeList blocks = switch_get_blocks(xmlnode);
  for (XMLNode block_node : blocks) {
    Block* block = new Block(block_node, this, ast);
    m_blks.push_back(block);
  }
  // m_cond = switch_get_condition(xmlnode);
  // m_cond = xmlnode.child("condition").child("expr");
  m_children.insert(m_children.end(), m_blks.begin(), m_blks.end());
}

void Switch::GetCode(std::set<ASTNode*> nodes,
                     std::string &ret, bool all, bool add_brace) {
  // if (!nodes.empty() && nodes.count(this) == 0) return;
  bool selected = nodes.count(this) == 1;
  selected |= all;
  ret.size();
  return;
}

Case::Case(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  m_parent = parent;
  m_xmlnode = xmlnode;
  m_ast = ast;
  // FIXME Should assert(m_parent!=NULL) ?
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    // FIXME symbol table do not change
    m_sym_tbl = m_parent->GetSymTbl();
  }
}
void Case::GetCode(std::set<ASTNode*> nodes,
                   std::string &ret, bool all, bool add_brace) {
  bool selected = nodes.count(this) == 1;
  selected |= all;
}

Default::Default(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  m_parent = parent;
  m_xmlnode = xmlnode;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_parent->GetSymTbl();
  }
}
void Default::GetCode(std::set<ASTNode*> nodes,
                      std::string &ret, bool all, bool add_brace) {
}

/*******************************
 ** Loop
 *******************************/

While::While(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymTbl());
  }
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- WHILE" << "\n";
  #endif
  XMLNode blk_node = while_get_block(xmlnode);
  m_blk = new Block(blk_node, this, ast);
  m_cond = while_get_condition_expr(xmlnode);
  m_children.push_back(m_blk);
}

void While::GetCode(std::set<ASTNode*> nodes,
                    std::string &ret, bool all, bool add_brace) {
  // if (!nodes.empty() && nodes.count(this) == 0) return;
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "while (";
    ret += get_text(m_cond);
    ret += ")";
  }
  m_blk->GetCode(nodes, ret, all, selected);
}

std::string While::GetLabel() {
  std::string ret;
  ret += "while (";
  ret += get_text(m_cond);
  ret += ")";
  return ret;
}



Do::Do(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymTbl());
  }
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- DO" << "\n";
  #endif
  XMLNode blk_node = while_get_block(xmlnode);
  m_blk = new Block(blk_node, this, ast);
  m_cond = while_get_condition_expr(xmlnode);
  m_children.push_back(m_blk);
}

void Do::GetCode(std::set<ASTNode*> nodes,
                 std::string &ret, bool all, bool add_brace) {
  // if (!nodes.empty() && nodes.count(this) == 0) return;
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "do ";
  }
  m_blk->GetCode(nodes, ret, all, selected);
  if (selected) {
    ret += "while (";
    ret += get_text(m_cond);
    ret += ");";
  }
}

std::string Do::GetLabel() {
  std::string ret;
  ret += "do ";
  ret += "while (";
  ret += get_text(m_cond);
  ret += ");";
  return ret;
}


For::For(XMLNode xmlnode, ASTNode* parent, AST *ast) {
  m_xmlnode = xmlnode;
  m_parent = parent;
  m_ast = ast;
  if (m_parent == NULL) {
    // this is root, create the default symbol table.
    m_sym_tbl = m_ast->CreateSymTbl(NULL);
  } else {
    m_sym_tbl = m_ast->CreateSymTbl(m_parent->GetSymTbl());
  }
  #ifdef DEBUG_AST_NODE_TRACE
  std::cout << "---- FOR" << "\n";
  #endif
  m_cond = for_get_condition_expr(xmlnode);
  m_incr = for_get_incr_expr(xmlnode);
  XMLNode blk_n = for_get_block(xmlnode);
  m_blk = new Block(blk_n, this, ast);
  m_inits = for_get_init_decls_or_exprs(xmlnode);
  m_children.push_back(m_blk);
  /**
   * Push the symbol table
   */
  for (XMLNode init : m_inits) {
    if (kind(init) == NK_Decl) {
      std::string name = decl_get_name(init);
      m_sym_tbl->AddSymbol(name, this);
    }
  }
}

void For::GetCode(std::set<ASTNode*> nodes,
                  std::string &ret, bool all, bool add_brace) {
  // if (!nodes.empty() && nodes.count(this) == 0) return;
  bool selected = nodes.count(this) == 1;
  selected |= all;
  if (selected) {
    ret += "for (";
    // init
    if (!m_inits.empty()) {
      for (XMLNode n : m_inits) {
        ret += get_text(n);
        ret += ",";
      }
      ret.pop_back();
    }
    ret += ";" + get_text(m_cond) + ";" + get_text(m_incr) + ")";
  }
  m_blk->GetCode(nodes, ret, all, selected);
}

std::string For::GetLabel() {
  std::string ret;
  ret += "for (";
  // init
  if (!m_inits.empty()) {
    for (XMLNode n : m_inits) {
      ret += get_text(n);
      ret += ",";
    }
    ret.pop_back();
  }
  ret += ";" + get_text(m_cond) + ";" + get_text(m_incr) + ")";
  return ret;
}

std::set<std::string> For::GetVarIds() {
  std::set<std::string> ret;
  std::set<std::string> ids;
  ids = get_var_ids(m_cond);
  ret.insert(ids.begin(), ids.end());
  for (XMLNode init : m_inits) {
    ids = get_var_ids(init);
    ret.insert(ids.begin(), ids.end());
  }
  ids = get_var_ids(m_incr);
  ret.insert(ids.begin(), ids.end());
  return ret;
}


std::set<std::string> For::GetIdToResolve() {
  std::set<std::string> ret;
  std::set<std::string> tmp;
  tmp = extract_id_to_resolve(get_text(m_cond));
  ret.insert(tmp.begin(), tmp.end());
  tmp = extract_id_to_resolve(get_text(m_incr));
  ret.insert(tmp.begin(), tmp.end());
  for (XMLNode n : m_inits) {
    tmp = extract_id_to_resolve(get_text(n));
    ret.insert(tmp.begin(), tmp.end());
  }
  return ret;
}

/**
 * Disable because it will not check something.
 * It just run some code, visualize it.
 */
TEST(ASTNodeTestCase, DISABLED_NodeTest) {
  ast::Doc doc;
const char *raw = R"prefix(

int foo() {
if (x>0) {
  while (x<10) {
    a=b;
    c=d;
    if (a>c) {
      sum+=c;
    } else if (a==c) {
      sum += con1;
    } else {
      sum += a;
    }
  }
} else {
  sum = 0;
  for (int i=0;i<8;i++) {
    sum += i;
  }
}
}

)prefix";

 utils::string2xml(raw, doc);
 NodeList nodes = find_nodes(doc, NK_Function);
 ASSERT_EQ(nodes.size(), 1);
 AST *ast = new AST(nodes[0]);
 std::string code = ast->GetCode();
 // code = utils::exec_in("indent", code.c_str());
 std::cout <<code  << "\n";
// THIS IS IMPORTANT! SEED the random.
 utils::seed_rand();
 Gene gene, cgene;
 std::string dot;

 std::cout <<"begin test suite"  << "\n";

 /**
  * Suite 1
  */
 gene.Rand(ast->size());
 // gene.SetFlat("0000111011101010010110000");
 std::cout <<"gene: ";
 gene.dump();
 cgene = ast->CompleteGene(gene);
 std::cout <<"cgene: ";
 cgene.dump();
 dot = ast->VisualizeI(gene.GetIndiceS(), cgene.GetIndiceS());
 utils::visualize_dot_graph(dot);

 /**
  * Suite 2
  */
 // gene.Rand(ast->size());
 // std::cout <<"gene: ";
 // gene.dump();
 // cgene = ast->CompleteGene(gene);
 // std::cout <<"cgene: ";
 // cgene.dump();
 // dot = ast->VisualizeI(gene.GetIndiceS(), cgene.GetIndiceS());
 // utils::visualize_dot_graph(dot);

 // std::cout <<dot  << "\n";
 delete ast;
}

TEST(ASTNodeTestCase, DISABLED_ExtraNodeTest) {
  ast::Doc doc;
  const char *raw = R"prefix(
int
res_hnok(const char *dn) {
	int ppch = '\0', pch = PERIOD, ch = *dn++;

	while (ch != '\0') {
		int nch = *dn++;

		if (periodchar(ch)) {
			(void)NULL;
		} else if (periodchar(pch)) {
			if (!borderchar(ch))
				return (0);
		} else if (periodchar(nch) || nch == '\0') {
			if (!borderchar(ch))
				return (0);
		} else {
			if (!middlechar(ch))
				return (0);
		}
		ppch = pch, pch = ch, ch = nch;
	}
	return (1);
}
)prefix";

  utils::string2xml(raw, doc);
  NodeList nodes = find_nodes(doc, NK_Function);
  ASSERT_EQ(nodes.size(), 1);
  AST *ast = new AST(nodes[0]);
  ast->Visualize();
}

TEST(ASTNodeTestCase, DISABLED_VarDefUseTest) {
  ast::Doc doc;
  const char *raw = R"prefix(


int foo() {
  int x=0;
  int sum=0;
  if (x>0) {
    int a=1;
    int b=1;
    int c=2;
    int d=3;
    while (x<10) {
      a=b;
      c=d;
      int con1=8;
      if (a>c) {
        sum+=c;
      } else if (a==c) {
        sum += con1;
      } else {
        sum += a;
      }
    }
  } else {
    sum = 0;
    for (int i=0;i<8;i++) {
      sum += i;
    }
  }
  return sum;
}
)prefix";

  utils::string2xml(raw, doc);
  NodeList nodes = find_nodes(doc, NK_Function);
  ASSERT_EQ(nodes.size(), 1);
  AST *ast = new AST(nodes[0]);
  std::string code = ast->GetCode();
  // code = utils::exec_in("indent", code.c_str());
  std::cout <<code  << "\n";
  // THIS IS IMPORTANT! SEED the random.
  utils::seed_rand();
  Gene gene, cgene;
  std::string dot;

  std::cout <<"begin test suite"  << "\n";

  /**
   * Suite 1
   */
  gene.Rand(ast->size());
  std::cout <<"gene: ";
  gene.dump();
  // cgene = ast->CompleteGene(gene);
  // gene.SetFlat("10000110000001010100010010000000");
  cgene = ast->CompleteVarDefUse(gene);
  std::cout <<"cgene: ";
  cgene.dump();
  dot = ast->VisualizeI(gene.GetIndiceS(), cgene.GetIndiceS());
  // utils::visualize_dot_graph(dot);

  /**
   * Suite 2
   */
  // gene.Rand(ast->size());
  // std::cout <<"gene: ";
  // gene.dump();
  // cgene = ast->CompleteGene(gene);
  // std::cout <<"cgene: ";
  // cgene.dump();
  // dot = ast->VisualizeI(gene.GetIndiceS(), cgene.GetIndiceS());
  // utils::visualize_dot_graph(dot);

  // std::cout <<dot  << "\n";
  delete ast;
}
