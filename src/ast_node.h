#ifndef AST_NODE_H
#define AST_NODE_H

#include "common.h"
#include "ast.h"

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
    std::vector<int> GetFlat() {return m_flat;}
    std::vector<int> GetIndice() {return m_indice;}
    std::set<int> GetIndiceS() {return m_indice_s;}
    /**
     * Setters
     */
    void SetFlat(std::vector<int> flat) {
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
    void SetFlat(std::string s) {
      std::vector<int> flat;
      for (char c : s) {
        if (c == '1') flat.push_back(1);
        else flat.push_back(0);
      }
      SetFlat(flat);
    }
    void SetIndice(std::vector<int> indice, size_t size) {
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
    void SetIndiceS(std::set<int> indice_s, size_t size) {
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
    void Rand(size_t size);
    size_t size() {return m_size;}
    void dump();
  private:
    size_t m_size = 0;
    std::vector<int> m_flat; // (0, 1, 1, 1, 0)
    std::vector<int> m_indice; // (0, 4, 5, 6, 8)
    std::set<int> m_indice_s; // (set of above)
  };

  class SymbolTable {
  public:
    /**
     * SymbolTable can only be created with the parent, i.e. the last level table.
     */
    SymbolTable(SymbolTable *parent) : m_parent(parent) {}
    SymbolTable *GetParent() {return m_parent;}
    /**
     * Look up recursively to parent, until empty
     */
    ASTNode* LookUp(std::string key) {
      ASTNode *ret;
      SymbolTable *tbl = this;
      while (tbl) {
        ret = tbl->LocalLookUp(key);
        if (ret) return ret;
        tbl = tbl->GetParent();
      }
      return NULL;
    }
    ASTNode *LocalLookUp(std::string key) {
      if (m_map.count(key) == 1) return m_map[key];
      else return NULL;
    }
    void AddSymbol(std::string key, ASTNode* value) {
      m_map[key] = value;
    }
    void dump();
    void local_dump();
  private:
    SymbolTable *m_parent = NULL;
    std::map<std::string, ASTNode*> m_map;
  };

  /**
   * This class represent the AST associated with an XML node.
   * The instance of this class will hold the memory for all the actually nodes(ASTNode*)
   * The deconstructor will deallocate all the ASTNode*
   */
  class AST {
  public:
    AST() {}
    AST(ast::XMLNode xmlnode);
    ~AST();
    /**
     * Visualization
     */
    std::string Visualize();
    std::string VisualizeI(std::set<int> bold_s, std::set<int> fill_s);
    std::string VisualizeN(std::set<ASTNode*> bold_s, std::set<ASTNode*> fill_s);
    /**
     * Code
     */
    std::string GetCode(std::set<ASTNode*> &nodes);
    size_t size() {return m_nodes.size();}
    std::string GetCode() {
      std::set<ASTNode*> s;
      return GetCode(s);
    }
    /**
     * Least Common Ancestor(LCA) Related
     */
    ASTNode *ComputeLCA(std::set<int> indices);
    ASTNode *ComputeLCA(std::set<ASTNode*> nodes);
    std::set<ASTNode*> CompleteGene(std::set<ASTNode*>);
    Gene CompleteGene(Gene gene);
    
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
  private:
    std::vector<ASTNode*> getPath(ASTNode *node, ASTNode* lca);
    size_t dist(ASTNode* low, ASTNode* high);
    int computeLvl(ASTNode* node);
    ASTNode* m_root = NULL;
    std::vector<ASTNode*> m_nodes;
    std::map<ASTNode*, int> m_idx_m;
    std::vector<SymbolTable*> m_sym_tbls; // just a storage
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
     * virtual std::string GetCode(std::set<ASTNode*> &nodes) = 0;
     * If 'nodes' is empty, get all code, as if the nodes set in above method contains all nodes
     * because it make no sense to get code for an empty set of allowed nodes, which should simply be empty.
     */
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) = 0;
    virtual std::set<std::string> GetVarIds() {return {};}

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
    iterator children_begin() {return m_children.begin();}
    iterator children_end() {return m_children.end();}
    // int Index() {return m_index;}
    ast::XMLNode GetXMLNode() {return m_xmlnode;}
    // should never set symbol table explicitly
    // void SetSymbolTable(SymbolTable *tbl) {m_sym_tbl = tbl;}
    SymbolTable *GetSymTbl() {return m_sym_tbl;}
  protected:
    XMLNode m_xmlnode;
    ASTNode *m_parent = NULL;
    std::vector<ASTNode*> m_children;
    // int m_index;
    AST *m_ast;
    SymbolTable *m_sym_tbl = NULL;
  };

  /**
   * <decl></decl>
   */
  class Decl {
  public:
    Decl(XMLNode n);
    std::string GetType() {return m_type;}
    std::string GetName() {return m_name;}
  private:
    XMLNode m_xmlnode;
    std::string m_type;
    std::string m_name;
    // TODO m_dimension
  };

  // function
  class Function : public ASTNode {
  public:
    Function(XMLNode n, ASTNode* parent, AST *ast);
    ~Function();
    virtual ASTNodeKind Kind() override {return ANK_Function;}
    virtual std::string GetLabel() override {return m_name;}
    std::string GetReturnType() {return m_ret_ty;}
    std::string GetName() {return m_name;}
    void GetParams() {}
    Block* GetBlock() {return m_blk;}
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
  private:
    std::string m_ret_ty;
    std::string m_name;
    Block *m_blk = NULL;
    std::vector<Decl*> m_params;
  };
  // general
  class Block : public ASTNode {
  public:
    Block(XMLNode, ASTNode* parent, AST *ast);
    ~Block() {}
    ASTNodeKind Kind() override {return ANK_Block;}
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::string GetLabel() override {return "Block";}
  private:
  };

  class Stmt : public ASTNode {
  public:
    Stmt(XMLNode, ASTNode* parent, AST *ast);
    virtual ~Stmt() {}
    ASTNodeKind Kind() override {return ANK_Stmt;}
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::set<std::string> GetVarIds() override {
      std::set<std::string> ids = get_var_ids(m_xmlnode);
      return ids;
    }
    virtual std::string GetLabel() override {
      std::string ret;
      std::set<ASTNode*> nodes;
      GetCode(nodes, ret);
      return ret;
    }
  private:
  };
  class Expr : public ASTNode {
  public:
    Expr(XMLNode, ASTNode* parent, AST *ast);
    ~Expr() {}
    ASTNodeKind Kind() override {return ANK_Expr;}
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::set<std::string> GetVarIds() override {
      return get_var_ids(m_xmlnode);
    }
    virtual std::string GetLabel() override {
      std::string ret;
      std::set<ASTNode*> nodes;
      GetCode(nodes, ret);
      return ret;
    }
  private:
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
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::string GetLabel() override {return "If";}
    virtual std::set<std::string> GetVarIds() override {
      return get_var_ids(m_cond);
    }
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
    Block* GetBlock() {return m_blk;}
    ~Else() {}
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::string GetLabel() override {return "Else";}
  private:
    Block *m_blk = NULL;
  };
  
  class ElseIf : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_ElseIf;}
    ElseIf(XMLNode, ASTNode* parent, AST *ast);
    XMLNode GetCondition() {return m_cond;}
    Block* GetBlock() {return m_blk;}
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::string GetLabel() override {return "ElseIf";}
    virtual std::set<std::string> GetVarIds() override {
      return get_var_ids(m_cond);
    }
  private:
    XMLNode m_cond;
    Block *m_blk = NULL;
  };
  
  class Then : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Then;}
    Then(XMLNode, ASTNode* parent, AST *ast);
    Block* GetBlock() {return m_blk;}
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::string GetLabel() override {return "Then";}
  private:
    Block *m_blk = NULL;
  };
  
  class Switch : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Switch;}
    Switch(XMLNode, ASTNode* parent, AST *ast);
    ~Switch() {}
    // XMLNode Condition() {return m_cond;}
    // void Cases() {}
    std::vector<Block*> GetBlocks() {return m_blks;}
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::string GetLabel() override {return "Switch";}
  private:
    // XMLNode m_cond;
    std::vector<Block*> m_blks;
  };
  /**
   * Case is currently not used.
   */
  class Case : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Case;}
    Case(XMLNode, ASTNode* parent, AST *ast);
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::string GetLabel() override {return "Case";}
  private:
  };
  class Default : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Default;}
    Default(XMLNode, ASTNode* parent, AST *ast);
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::string GetLabel() override {return "Default";}
  private:
  };

  // loop
  class For : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_For;}
    For(XMLNode, ASTNode* parent, AST *ast);
    ~For() {}
    XMLNodeList GetInits() {return m_inits;}
    XMLNode GetCondition() {return m_cond;}
    XMLNode GetIncr() {return m_incr;}
    Block* GetBlock() {return m_blk;}
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::string GetLabel() override {return "For";}
    virtual std::set<std::string> GetVarIds() override {
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
  private:
    XMLNode m_cond;
    XMLNodeList m_inits;
    XMLNode m_incr;
    Block* m_blk = NULL;
  };
  class While : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_While;}
    While(XMLNode, ASTNode* parent, AST *ast);
    ~While() {}
    XMLNode GetCondition() {return m_cond;}
    Block* GetBlock() {return m_blk;}
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::string GetLabel() override {return "While";}
    virtual std::set<std::string> GetVarIds() override {
      return get_var_ids(m_cond);
    }
  private:
    XMLNode m_cond;
    Block *m_blk = NULL;
  };
  class Do : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Do;}
    Do(XMLNode, ASTNode* parent, AST *ast);
    ~Do() {}
    XMLNode GetCondition() {return m_cond;}
    Block* GetBlock() {return m_blk;}
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::string GetLabel() override {return "Do";}
    virtual std::set<std::string> GetVarIds() override {
      return get_var_ids(m_cond);
    }
  private:
    XMLNode m_cond;
    Block *m_blk = NULL;
  };

  class ASTOther : public ASTNode {
  public:
    ASTNodeKind Kind() override {return ANK_Other;}
    ASTOther(XMLNode, ASTNode* parent, AST *ast);
    virtual void GetCode(std::set<ASTNode*> &nodes, std::string &ret) override;
    virtual std::string GetLabel() override {return "Other";}
  private:
  };

} // end namespace ast
#endif /* AST_NODE_H */
