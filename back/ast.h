#ifndef AST_H
#define AST_H

#include "helium/utils/common.h"
#include "helium/parser/xmlnode.h"
#include "helium/type/type.h"
#include "helium/type/variable.h"

class ASTNode;
class AST;

/**
 * The gene. It has a size. It should associcate with an AST.
 */
class Gene {
public:
  Gene(AST *ast);
  ~Gene() {}
  std::vector<int> GetFlat() {return m_flat;}
  std::vector<int> GetIndice() {return m_indice;}
  std::set<int> GetIndiceS() {return m_indice_s;}
  // setters
  void SetIndiceS(std::set<int> indice_s, size_t size);
  void SetIndice(std::vector<int> indice, size_t size);
  void SetFlat(std::string s);
  void SetFlat(std::vector<int> flat);
  // other
  std::set<ASTNode*> ToASTNodeSet();
  // random
  void Rand();
  void LeafRand();
  size_t size() {return m_size;}
  size_t leaf_size();
  // all selected nodes
  size_t node_size() {return m_indice.size();}
  void dump();
  void AddNode(ASTNode *node);
  bool HasIndex(int idx) {
    return m_indice_s.count(idx) == 1;
  }
private:
  size_t m_size = 0;
  std::vector<int> m_flat; // (0, 1, 1, 1, 0)
  std::vector<int> m_indice; // (0, 4, 5, 6, 8)
  std::set<int> m_indice_s; // (set of above)
  AST *m_ast=NULL;
};

/**
 * TODO NOW Use Variable class for this value
 */
class SymbolTableValue {
public:
  SymbolTableValue(std::string symbol_name, Type *type, ASTNode *node)
    :m_name(symbol_name), m_type(type), m_node(node) {}
  ~SymbolTableValue() {}
  std::string GetName() {return m_name;}
  Type* GetType() {return m_type;}
  ASTNode *GetNode() {return m_node;}
private:
  std::string m_name;
  // this type is not created here. // it is actually created inside the AST Node models
  // e.g. The type created in class "Decl". It will also be free-d there.
  // so make sure that ASTNode class still alive when using this type pointer
  Type *m_type;
  ASTNode *m_node = NULL;
};

class SymbolTable {
public:
  /**
   * SymbolTable can only be created with the parent, i.e. the last level table.
   */
  SymbolTable(SymbolTable *parent) : m_parent(parent) {}
  ~SymbolTable() {
    for (auto m : m_map) {
      delete m.second;
    }
  }
  SymbolTable *GetParent() {return m_parent;}
  /**
   * Look up recursively to parent, until empty
   */
  SymbolTableValue* LookUp(std::string key) {
    SymbolTableValue *ret;
    SymbolTable *tbl = this;
    while (tbl) {
      ret = tbl->LocalLookUp(key);
      if (ret) return ret;
      tbl = tbl->GetParent();
    }
    return NULL;
  }
  SymbolTableValue *LocalLookUp(std::string key) {
    if (m_map.count(key) == 1) return m_map[key];
    else return NULL;
  }

  /**
   * key is name of the variable(symbol)
   */
  void AddSymbol(std::string key, Type *type, ASTNode* node) {
    if (m_map.count(key) == 1) {
      delete m_map[key];
    }
    // FIXME where to free these?
    m_map[key] = new SymbolTableValue(key, type, node);
  }
  void dump();
  void local_dump();
  std::string ToString();
  std::string ToStringLocal();
private:
  SymbolTable *m_parent = NULL;
  std::map<std::string, SymbolTableValue*> m_map;
};



class ASTFactory {
public:
  /**
   * Create AST from a XML document.
   * Use the first function node available.
   * Return NULL if no <function> node found.
   */
  // static AST* CreateASTFromDoc(ast::XMLDoc *doc);
};

/**
 * This class represent the AST associated with an XML node.
 * The instance of this class will hold the memory for all the actually nodes(ASTNode*)
 * The deconstructor will deallocate all the ASTNode*

 * I'm going to get a light weight AST
 * The AST should, by default, only construct the structure
 * It then provide options for:
 * - Create Symbol Table
 * - Decoration of input and output
 */
class AST {
public:
  // FIXME should I remove this constructor?
  // AST() {}
  AST(XMLNode xmlnode);
  ~AST();
  size_t size() {return m_nodes.size();}
  size_t leaf_size();

  ASTNode *GetRoot() {return m_root;}
  std::vector<ASTNode*> GetNodes() {return m_nodes;}

  /**
   * Create symbol table for the whole AST
   * The constructor will NOT create symbol table by default.
   */
  void CreateSymbolTable();
    
  /**
   * Visualization
   */
  std::string Visualize(std::string filename="out");
  std::string VisualizeI(std::set<int> yellow_s, std::set<int> cyan_s, std::string filename="out");
  std::string VisualizeN(std::set<ASTNode*> yellow_s, std::set<ASTNode*> cyan_s, std::string filename="out");
  std::string VisualizeSlice(std::string filename="out");

  void Visualize2(bool open=true);

  
  /**
   * Code
   */
  std::string GetCode(std::set<ASTNode*> nodes);
  std::string GetCode(std::set<int> indice) {
    return GetCode(Index2Node(indice));
  }
  std::string GetCode();
  std::string GetFilename();
  int GetLineNumber();

  /**
   * The first callsite (or random one because I have a set)
   */
  ASTNode* GetCallSite(std::string func_name);
  std::set<ASTNode*> GetCallSites(std::string func_name);

  void SetSlice();
  void ClearSlice();

  std::set<ASTNode*> GetLeafNodes();

  /**
   *  set the decl, so that when getting nodes, every ASTNode can check if some extra definition should be outputed
   * Only when the node is not selected, it has the chance to output some extra information, i.e. the declaration of a variable.
   */
  void SetDecl(std::map<ASTNode*, std::set<std::string> > decl_input_m,
               std::map<ASTNode*, std::set<std::string> > decl_m) {
    m_decl_input_m = decl_input_m;
    m_decl_m = decl_m;
  }
  void SetDecoDecl(std::map<ASTNode*, std::set<std::string> > decl_m) {
    m_decl_m = decl_m;
  }
  void SetDecoDeclInput(std::map<ASTNode*, std::set<std::string> > decl_input_m) {
    m_decl_input_m = decl_input_m;
  }
  /**
   * Map from the Node, right before which to output, the variable names
   */
  void SetDecoOutput(std::map<ASTNode*, std::set<std::string> > deco_output_m) {
    // TODO need to remove this?
    m_deco_output_m = deco_output_m;
  }
  /**
   * Use the new type system.
   */
  void SetOutput(std::map<ASTNode*, std::vector<Variable> > output_m) {
    m_output_m = output_m;
  }
  void ClearOutput() {
    m_output_m.clear();
  }
  void ClearDecl() {
    m_decl_input_m.clear();
    m_decl_m.clear();
  }
  void HideOutput() {
    m_output_back_m = m_output_m;
    m_output_m.clear();
  }
  void RestoreOutput() {
    m_output_m = m_output_back_m;
    m_output_back_m.clear();
  }

  std::set<std::string> GetRequiredDecl(ASTNode *node) {
    if (m_decl_m.count(node) == 1) return m_decl_m[node];
    else return {};
  }
  std::set<std::string> GetRequiredDeclWithInput(ASTNode *node) {
    if (m_decl_input_m.count(node) == 1) return m_decl_input_m[node];
    else return {};
  }
  std::set<std::string> GetRequiredDecoOutput(ASTNode* node) {
    if (m_deco_output_m.count(node) == 1) return m_deco_output_m[node];
    else return {};
  }
  std::vector<Variable> GetRequiredOutputVariables(ASTNode *node) {
    if (m_output_m.count(node) == 1) return m_output_m[node];
    else return {};
  }
  /**
   * Assume the root of AST is a function.
   * Get that function name
   */
  std::string GetFunctionName();
  /**
   * Least Common Ancestor(LCA) Related
   */
  ASTNode *ComputeLCA(std::set<int> indices);
  ASTNode *ComputeLCA(std::set<ASTNode*> nodes);
  ASTNode *ComputeLCA(Gene *g) {
    assert(g);
    return ComputeLCA(g->GetIndiceS());
  }

  int Distance(ASTNode* m, ASTNode* n) {
    ASTNode *lca = ComputeLCA(std::set<ASTNode*> {m,n});
    assert(lca);
    int ret = 0;
    ret += getPath(m, lca).size();
    ret += getPath(n, lca).size();
    return ret;
  }

  /********************************
   * Node set operation
   *******************************/
  std::set<ASTNode*> CompleteGene(std::set<ASTNode*>);
  /**
   * Will complete gene, and modify in place.
   * Return the set of nodes newly add by this completion
   */
  std::set<ASTNode*> CompleteGene(Gene *gene);
  std::set<ASTNode*> CompleteGeneToRoot(std::set<ASTNode*> gene);

  std::set<ASTNode*> PatchGrammar(std::set<ASTNode*> nodes);

  /**
   * Remove the root node.
   */
  std::set<ASTNode*> RemoveRoot(std::set<ASTNode*> nodes);
    
  /**
   * Helper function for set<ASTNode*> <-> set<int>
   */
  std::set<int> Node2Index(std::set<ASTNode*> nodes);
  std::vector<int> Node2Index(std::vector<ASTNode*> nodes);
  std::set<ASTNode*> Index2Node(std::vector<int> indices);
  std::set<ASTNode*> Index2Node(std::set<int> indices);
  /**
   * IO Resolving
   */
  /**
   * Return the nodes to be newly added, which is the input nodes.
   * We should recursively resolve,
   * to make sure the newly added nodes do not introduce new unresolved(undefined) variables
   */
  std::set<ASTNode*> CompleteVarDefUse(std::set<ASTNode*> nodes);
  Gene CompleteVarDefUse(Gene gene);
  /**
   * These symbol tables must be free'd in ~AST
   * TODO This is the only place to create a symbol table.
   * Consider some technique to enforce it.
   */
  // SymbolTable *CreateSymTbl(SymbolTable *parent) {
  //   SymbolTable *ret = new SymbolTable(parent);
  //   m_sym_tbls.push_back(ret);
  //   return ret;
  // }
  ASTNode* GetNodeByIndex(size_t idx) {
    if (idx >= m_nodes.size()) return NULL;
    return m_nodes[idx];
  }
  int GetIndexByNode(ASTNode* node) {
    if (m_idx_m.count(node) == 1) return m_idx_m[node];
    return -1; // not found
  }
  bool Contains(ASTNode* node) {
    return m_idx_m.count(node) == 1;
  }
  ASTNode* GetNodeByLinum(int linum);
  /**
   * Get ast node by the raw xml node
   */
  ASTNode* GetNodeByXMLNode(XMLNode xmlnode) {
    if (m_xmlnode_m.count(xmlnode) == 1) {
      return m_xmlnode_m[xmlnode];
    }
    return NULL;
  }
  /**
   * The input xmlnode can be any sub node under a ASTNode.
   * It will recursively search the parent of the xmlnode.
   * Until no further parent avaiable.
   */
  ASTNode* GetEnclosingNodeByXMLNode(XMLNode xmlnode) {
    while (xmlnode) {
      ASTNode *ret = GetNodeByXMLNode(xmlnode);
      if (ret) return ret;
      xmlnode = xmlnode.parent();
    }
    return NULL;
  }
  ASTNode *GetPreviousLeafNode(ASTNode *node);
  ASTNode *GetPreviousLeafNodeInSlice(ASTNode *node);
  /**
   * Get the first nodes AMONG the input nodes
   */
  ASTNode *GetFirstNode(std::set<ASTNode*> nodes) {
    // assert all input nodes are in.
    assert(!nodes.empty());
    for (ASTNode *node : nodes) {
      assert(Contains(node));
    }
    // get the minimum index
    int ret_idx = m_nodes.size();
    ASTNode *ret = NULL;
    for (ASTNode* node : nodes) {
      int idx = GetIndexByNode(node);
      if (ret_idx > idx) {
        ret_idx = idx;
        ret = node;
      }
    }
    return ret;
  }

  bool IsInFreedList(ASTNode *node) {
    assert(node);
    return m_freed_node.count(node) == 0;
  }
  std::vector<std::string> GetFreedExprs(ASTNode *node) {
    assert(node);
    std::vector<std::string> ret;
    if (m_freed_node.count(node) == 1) {
      ret = m_freed_node[node];
    }
    return ret;
  }

  void SetFailurePoint(ASTNode *astnode) {
    m_failure_points.insert(astnode);
  }
  void ClearFailurePoint(ASTNode *astnode) {
    // m_failure_points.clear();
    m_failure_points.erase(astnode);
  }
  bool IsFailurePoint(ASTNode *astnode) {
    return m_failure_points.count(astnode) == 1;
  }

  
private:
  std::vector<ASTNode*> getPath(ASTNode *node, ASTNode* lca);
  size_t dist(ASTNode* low, ASTNode* high);
  int computeLvl(ASTNode* node);
  ASTNode* m_root = NULL;
  XMLNode m_xmlnode; // the original xmlnode parsing in
  std::vector<ASTNode*> m_nodes;
  std::map<ASTNode*, int> m_idx_m;
  std::map<XMLNode, ASTNode*> m_xmlnode_m;
  // std::vector<SymbolTable*> m_sym_tbls; // just a storage
  // from the decl node to the variable needed to be declared
  std::map<ASTNode*, std::set<std::string> > m_decl_input_m;
  // do not need input
  std::map<ASTNode*, std::set<std::string> > m_decl_m;
  std::map<ASTNode*, std::set<std::string> > m_deco_output_m; // deprecated
  std::map<ASTNode*, std::vector<Variable> > m_output_m;
  std::map<ASTNode*, std::vector<Variable> > m_output_back_m;

  // slicing
  std::set<ASTNode*> m_slice; // nodes in the set of slice
  bool m_is_slice_active = false; // whether the slice is set, or cleared.
  // FIXME this should be set when creating the AST.
  // It can be empty, when I just want the AST, but then I can do nothing for the filename related operations, like slice mask
  std::string m_filename; // the filename current AST belongs

  // the free-d list instrumentation related
  std::map<ASTNode*, std::vector<std::string> > m_freed_node;


  std::set<ASTNode*> m_failure_points;
  
};


#endif /* AST_H */
