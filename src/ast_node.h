#ifndef AST_NODE_H
#define AST_NODE_H

#include "common.h"
#include "ast.h"

#include "type.h"
#include "new_type.h"

#include "resolver.h"
#include "slice_reader.h"

namespace ast {
  typedef enum {
    // function
    ANK_Function,
    // general
    ANK_Block,
    ANK_Stmt,
    ANK_Expr,
    // condition
    ANK_If,
    ANK_Then,
    ANK_Else,
    ANK_ElseIf,
    ANK_Switch,
    ANK_Case,
    ANK_Default,
    // loop
    ANK_While,
    ANK_For,
    ANK_Do,
    ANK_Other
  } ASTNodeKind;

  class AST;
  class ASTNode;
  class Function;
  class Block;
  class Stmt;
  class Expr;
  class If;
  class Then;
  class Else;
  class ElseIf;
  class Switch;
  class Case;
  class Default;
  class While;
  class For;
  class Do;
  class ASTOther;

  void dfs(ASTNode *root, std::vector<ASTNode*> &visited);

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
    SymbolTableValue(std::string symbol_name, NewType *type, ASTNode *node)
      :m_name(symbol_name), m_type(type), m_node(node) {}
    ~SymbolTableValue() {}
    std::string GetName() {return m_name;}
    NewType* GetType() {return m_type;}
    ASTNode *GetNode() {return m_node;}
  private:
    std::string m_name;
    // this type is not created here. // it is actually created inside the AST Node models
    // e.g. The type created in class "Decl". It will also be free-d there.
    // so make sure that ASTNode class still alive when using this type pointer
    NewType *m_type;
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
    void AddSymbol(std::string key, NewType *type, ASTNode* node) {
      if (m_map.count(key) == 1) {
        delete m_map[key];
      }
      // FIXME where to free these?
      m_map[key] = new SymbolTableValue(key, type, node);
    }
    void dump();
    void local_dump();
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
    static AST* CreateASTFromDoc(ast::XMLDoc *doc);
  };

  /**
   * This class represent the AST associated with an XML node.
   * The instance of this class will hold the memory for all the actually nodes(ASTNode*)
   * The deconstructor will deallocate all the ASTNode*
   */
  class AST {
  public:
    // FIXME should I remove this constructor?
    AST() {}
    AST(ast::XMLNode xmlnode);
    ~AST();
    size_t size() {return m_nodes.size();}
    size_t leaf_size();
    /**
     * Visualization
     */
    std::string Visualize(std::string filename="out");
    std::string VisualizeI(std::set<int> yellow_s, std::set<int> cyan_s, std::string filename="out");
    std::string VisualizeN(std::set<ASTNode*> yellow_s, std::set<ASTNode*> cyan_s, std::string filename="out");
    std::string VisualizeSlice(std::string filename="out");
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
    void SetOutput(std::map<ASTNode*, std::vector<NewVariable> > output_m) {
      m_output_m = output_m;
    }
    void ClearOutput() {
      m_output_m.clear();
    }
    void ClearDecl() {
      m_decl_input_m.clear();
      m_decl_m.clear();
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
    std::vector<NewVariable> GetRequiredOutputVariables(ASTNode *node) {
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
    SymbolTable *CreateSymTbl(SymbolTable *parent) {
      SymbolTable *ret = new SymbolTable(parent);
      m_sym_tbls.push_back(ret);
      return ret;
    }
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
  private:
    std::vector<ASTNode*> getPath(ASTNode *node, ASTNode* lca);
    size_t dist(ASTNode* low, ASTNode* high);
    int computeLvl(ASTNode* node);
    ASTNode* m_root = NULL;
    std::vector<ASTNode*> m_nodes;
    std::map<ASTNode*, int> m_idx_m;
    std::map<XMLNode, ASTNode*> m_xmlnode_m;
    std::vector<SymbolTable*> m_sym_tbls; // just a storage
    // from the decl node to the variable needed to be declared
    std::map<ASTNode*, std::set<std::string> > m_decl_input_m;
    // do not need input
    std::map<ASTNode*, std::set<std::string> > m_decl_m;
    std::map<ASTNode*, std::set<std::string> > m_deco_output_m; // deprecated
    std::map<ASTNode*, std::vector<NewVariable> > m_output_m;

    // slicing
    std::set<ASTNode*> m_slice; // nodes in the set of slice
    bool m_is_slice_active = false; // whether the slice is set, or cleared.
    // FIXME this should be set when creating the AST.
    // It can be empty, when I just want the AST, but then I can do nothing for the filename related operations, like slice mask
    std::string m_filename; // the filename current AST belongs
  };

  class ASTNodeFactory {
  public:
    static ASTNode* CreateASTNode(XMLNode xml_node, ASTNode *parent, AST *ast);
  };

  class ASTNode {
  public:
    typedef std::vector<ASTNode*>::iterator iterator;
    ASTNode() {}
    virtual ~ASTNode() {}
    virtual ASTNodeKind Kind() = 0;
    /**
     * Label is the string appear on the visualization dot graph.
     * E.g. for 'while', 'if', it is the name; for a single statement, it is the statement itself.
     */
    virtual std::string GetLabel() = 0;
    /**
     * code
     * get only the nodes in the set
     * I do not want this because I need to construct the string more efficiently
     * virtual std::string GetCode(std::set<ASTNode*> nodes) = 0;
     * If 'nodes' is empty, get all code, as if the nodes set in above method contains all nodes
     * because it make no sense to get code for an empty set of allowed nodes, which should simply be empty.
     */
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) = 0;
    virtual std::string dump() {
      std::string code;
      GetCode({}, code, true);
      return code;
    }
    // void GetCode(std::set<ASTNode*> nodes,
    //              std::string &ret, bool all,
    //              std::map<ASTNode*, std::set<std::string> > &decl_input_m,
    //              std::map<ASTNode*, std::set<std::string> > &decl_m) {
    //   // FIXME assert this node can only be in either nodes, decl_input_m, or decl_m
    //   if (decl_input_m.count(this) == 1) {
    //     GetDeclWithInput(decl_input_m[this], ret);
    //   }
    //   if (decl_m.count(this) == 1) {
    //     GetDecl(decl_m[this], ret);
    //   }
    //   GetCode(nodes, ret, all, decl_input_m, decl_m);
    // }
    /**
     * At this node, construct the decl statement for the variables in "names"
     * Can just look up symbol table, but need to make sure the variables defined in this node?
     */
    // virtual void GetDecl(std::set<std::string> names, std::string &ret) {
    //   ret += "/* default decl (empty) */\n";
    // }
    // /**
    //  * Not only the decl statement, but also give it input.
    //  */
    // virtual void GetDeclWithInput(std::set<std::string> names, std::string &ret) {
    //   GetDecl(names, ret);
    // }
    virtual std::set<std::string> GetVarIds() {return {};}
    virtual std::set<std::string> GetIdToResolve() {return {};}
    /**
     * Look Up the first definition of the variable "id"
     * That means it must appear on the left side of "="
     * How about structures?
     * If it turns out to depend on some other variables, simply the node
     * If it turns out to be constant, return this node.
     * To sum up, if itself is not on right side, simply return the node
     * If no record found, recursively try previous sibling or parent

     * So which node needs to override it?
     * stmt, for
     * DO NOT: if, elseif
     * Which nodes don't need?
     */
    virtual ASTNode* LookUpDefinition(std::string id) {
      if (this->PreviousSibling()) return this->PreviousSibling()->LookUpDefinition(id);
      else if (this->GetParent()) return this->GetParent()->LookUpDefinition(id);
      else return NULL;
    }

    // tree travesal related
    // DO NOT allow to set parent
    // must be given when create it
    // void SetParent(ASTNode *parent) {m_parent = parent;}
    ASTNode* GetParent() {return m_parent;}
    std::vector<ASTNode*> Children() {return m_children;}
    ASTNode *Child(int idx) {
      if (idx <0 || idx >= (int)m_children.size()) return NULL;
      return m_children[idx];
    }
    std::vector<ASTNode*> GetChildren() {return m_children;}
    int GetLevel() const {
      ASTNode *p = m_parent;
      int ret = 0;
      while (p) {
        p = p->GetParent();
        ret ++;
      }
      return ret;
    }

    AST* GetAST() {
      return m_ast;
    }

    ASTNode *PreviousSibling() {
      if (m_parent==NULL) return NULL;
      std::vector<ASTNode*> children = m_parent->GetChildren();
      ASTNode *ret=NULL;
      for (ASTNode *child : children) {
        if (child == this) return ret;
        ret = child;
      }
      // "this" must be a child of "m_parent"
      assert(false);
    }
    ASTNode* NextSibling() {
      if (m_parent==NULL) return NULL;
      std::vector<ASTNode*> children = m_parent->GetChildren();
      ASTNode *ret=NULL;
      std::reverse(children.begin(), children.end());
      for (ASTNode *child : children) {
        if (child == this) return ret;
        ret = child;
      }
      // "this" must be a child of "m_parent"
      assert(false);
    }
    iterator children_begin() {return m_children.begin();}
    iterator children_end() {return m_children.end();}
    // int Index() {return m_index;}
    ast::XMLNode GetXMLNode() {return m_xmlnode;}
    // should never set symbol table explicitly
    // void SetSymbolTable(SymbolTable *tbl) {m_sym_tbl = tbl;}
    SymbolTable *GetSymbolTable() {return m_sym_tbl;}
    int GetBeginLinum() {
      return get_node_line(m_xmlnode);
    }
    int GetEndLinum() {
      return get_node_last_line(m_xmlnode);
    }
  protected:
    XMLNode m_xmlnode;
    ASTNode *m_parent = NULL;
    std::vector<ASTNode*> m_children;
    // int m_index;
    AST *m_ast;
    SymbolTable *m_sym_tbl = NULL;
    // I'm not using these two fields because I need to initialize them in every Node class
    // Instead, the GetBeginLinum and GetEndLinum will compute them on-the-fly
    // this will of course bring some overhead
    // int m_begin_linum = 0;
    // int m_end_linum = 0;
  };



  /*******************************
   * Models
   *******************************/

  /**
   * some writeup

   Models that will add symbol table:
   1. Function
   2. Stmt
   3. For
   */

  
  /**
   * <decl></decl>
   */
  class Decl {
  public:
    Decl(XMLNode n);
    ~Decl();
    NewType *GetType() const {return m_type;}
    std::string GetName() const {return m_name;}
    // std::vector<std::string> GetDims() const {return m_dims;}
    // int GetDim() const {return m_dims.size();}
  private:
    XMLNode m_xmlnode;
    // std::string m_type;
    // TODO NOW use type in Decl, and others
    NewType *m_type = NULL;
    std::string m_name;
    // TODO m_dimension
    // this is dimensions
    // if the code is char buf[5][4], the result is a vector of string "5" "4"
    // if the dimension is [], it will have an empty string, but the size of m_dims is still valid
    // Actually I only intend to use the size here.
    // A design decision:
    // whether to use a fixed length of buffer (of the stack), or use a variable length of buffer (on the heap)
    // std::vector<std::string> m_dims;
  };

  // function
  class Function : public ASTNode {
  public:
    Function(XMLNode n, ASTNode* parent, AST *ast);
    ~Function();
    virtual ASTNodeKind Kind() override {return ANK_Function;}
    virtual std::string GetLabel() override;
    std::string GetReturnType() {return m_ret_ty;}
    std::string GetName() {return m_name;}
    void GetParams() {}
    virtual void GetCode(std::set<ASTNode*> nodes, std::string &ret, bool all) override;
    virtual std::set<std::string> GetIdToResolve() override;
    // virtual void GetDecl(std::set<std::string> names, std::string &ret) override {
    //   for (Decl *param : m_params) {
    //     std::string name =param->GetName();
    //     if (names.count(name) == 1) {
    //       ret += param->GetType() + " " + param->GetName() + ";\n";
    //     }
    //   }
    // }
  private:
    std::string m_ret_ty;
    std::string m_name;
    // Block *m_blk = NULL;
    std::vector<Decl*> m_params;
  };
  // general
  class Block : public ASTNode {
  public:
    Block(XMLNode, ASTNode* parent, AST *ast);
    ~Block() {}
    ASTNodeKind Kind() override {return ANK_Block;}
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::string GetLabel() override {return "Block";}
  private:
  };

  class Stmt : public ASTNode {
  public:
    Stmt(XMLNode, ASTNode* parent, AST *ast);
    virtual ~Stmt();
    ASTNodeKind Kind() override {return ANK_Stmt;}
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::set<std::string> GetVarIds() override {
      std::set<std::string> ids = get_var_ids(m_xmlnode);
      return ids;
    }
    virtual std::string GetLabel() override {
      std::string ret;
      std::set<ASTNode*> nodes;
      GetCode(nodes, ret, true);
      return ret;
    }
    virtual std::set<std::string> GetIdToResolve() override {
      std::string code;
      GetCode({}, code, true);
      return extract_id_to_resolve(code);
    }
    virtual ASTNode* LookUpDefinition(std::string id) override;
  private:
    std::vector<Decl*> m_decls;
  };

  // condition
  class If : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_If;}
    If(XMLNode xmlnode, ASTNode* parent, AST *ast);
    ~If() {}
    XMLNode GetCondition() {return m_cond;}
    Then* GetThen() {return m_then;}
    std::vector<ElseIf*> GetElseIfs() {return m_elseifs;}
    Else* GetElse() {return m_else;}
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::string GetLabel() override;
    virtual std::set<std::string> GetVarIds() override {
      return get_var_ids(m_cond);
    }
    virtual std::set<std::string> GetIdToResolve() override {
      return extract_id_to_resolve(get_text(m_cond));
    }
    // virtual ASTNode* LookUpDefinition(std::string id) override;
  private:
    XMLNode m_cond;
    Then *m_then = NULL;
    Else *m_else = NULL;
    std::vector<ElseIf*> m_elseifs;
  };


  class Else : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Else;}
    Else(XMLNode, ASTNode* parent, AST *ast);
    ~Else() {}
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::string GetLabel() override {return "Else";}
  private:
  };
  
  class ElseIf : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_ElseIf;}
    ElseIf(XMLNode, ASTNode* parent, AST *ast);
    XMLNode GetCondition() {return m_cond;}
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::string GetLabel() override;
    virtual std::set<std::string> GetVarIds() override {
      return get_var_ids(m_cond);
    }
    virtual std::set<std::string> GetIdToResolve() override {
      return extract_id_to_resolve(get_text(m_cond));
    }
    // virtual ASTNode* LookUpDefinition(std::string id) override;
  private:
    XMLNode m_cond;
  };
  
  class Then : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Then;}
    Then(XMLNode, ASTNode* parent, AST *ast);
    Block* GetBlock() {return m_blk;}
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::string GetLabel() override {return "Then";}
  private:
    Block *m_blk = NULL;
  };


  /*******************************
   * Swith
   *******************************/
  class Switch : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Switch;}
    Switch(XMLNode, ASTNode* parent, AST *ast);
    ~Switch() {}
    XMLNode GetCondition() {return m_cond;}
    std::vector<Case*> GetCases() {return m_cases;}
    Default* GetDefault() {return m_default;}
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::string GetLabel() override;
    virtual std::set<std::string> GetVarIds() override {
      return get_var_ids(m_cond);
    }
  private:
    XMLNode m_cond;
    std::vector<Case*> m_cases;
    Default *m_default = NULL;
  };

  class Case : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Case;}
    Case(XMLNode, ASTNode* parent, AST *ast);
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::string GetLabel() override;
  private:
    XMLNode m_cond;
  };
  class Default : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Default;}
    Default(XMLNode, ASTNode* parent, AST *ast);
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::string GetLabel() override {return "Default";}
  private:
  };

  // loop
  class For : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_For;}
    For(XMLNode, ASTNode* parent, AST *ast);
    ~For();
    XMLNodeList GetInits() {return m_inits;}
    XMLNode GetCondition() {return m_cond;}
    XMLNode GetIncr() {return m_incr;}
    // Block* GetBlock() {return m_blk;}
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::string GetLabel() override;
    virtual std::set<std::string> GetVarIds() override;
    virtual std::set<std::string> GetIdToResolve() override;
    virtual ASTNode* LookUpDefinition(std::string id) override;
  private:
    XMLNode m_cond;
    XMLNodeList m_inits;
    XMLNode m_incr;
    // Block* m_blk = NULL;
    std::vector<Decl*> m_decls;
  };
  class While : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_While;}
    While(XMLNode, ASTNode* parent, AST *ast);
    ~While() {}
    XMLNode GetCondition() {return m_cond;}
    // Block* GetBlock() {return m_blk;}
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::string GetLabel() override;
    virtual std::set<std::string> GetVarIds() override {
      return get_var_ids(m_cond);
    }
    virtual std::set<std::string> GetIdToResolve() override {
      return extract_id_to_resolve(get_text(m_cond));
    }
  private:
    XMLNode m_cond;
    // Block *m_blk = NULL;
  };
  class Do : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Do;}
    Do(XMLNode, ASTNode* parent, AST *ast);
    ~Do() {}
    XMLNode GetCondition() {return m_cond;}
    // Block* GetBlock() {return m_blk;}
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::string GetLabel() override;
    virtual std::set<std::string> GetVarIds() override {
      return get_var_ids(m_cond);
    }
    virtual std::set<std::string> GetIdToResolve() override {
      return extract_id_to_resolve(get_text(m_cond));
    }
  private:
    XMLNode m_cond;
    // Block *m_blk = NULL;
  };

  class ASTOther : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Other;}
    ASTOther(XMLNode, ASTNode* parent, AST *ast);
    virtual void GetCode(std::set<ASTNode*> nodes,
                         std::string &ret, bool all) override;
    virtual std::string GetLabel() override {return "Other";}
  private:
  };

} // end namespace ast
#endif /* AST_NODE_H */
