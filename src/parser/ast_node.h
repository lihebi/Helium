#ifndef AST_NODE_H
#define AST_NODE_H

#include "common.h"
#include "xmlnode.h"
#include "xmlnode_helper.h"
#include "type/type.h"
// #include "type/variable.h"
#include "resolver/resolver.h"
#include "parser/slice_reader.h"

#include "ast.h"

typedef enum {
  // function
  ANK_Function,
  ANK_Block,
  // general
  ANK_Stmt,
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

ASTNodeKind xmlnode_kind_to_astnode_kind(XMLNodeKind kind);

class ASTOption {
public:
  // ONLY for calculating segment size purpose
  static void TurnOffPOIInstrument() {m_instrument=false;}
  static void TurnOnPOIInstrument() {m_instrument=true;}
  static bool UsePOIInstrument() {return m_instrument;}
private:
  static bool m_instrument;
};


/**
 * <decl></decl>
 */
class Decl {
public:
  // Decl(XMLNode n);
  Decl(Type *t, std::string name, XMLNode node) : m_type(t), m_name(name), m_xmlnode(node) {}
  ~Decl();
  Type *GetType() const {return m_type;}
  std::string GetName() const {return m_name;}
  // std::vector<std::string> GetDims() const {return m_dims;}
  // int GetDim() const {return m_dims.size();}
private:
  // std::string m_type;
  // TODO NOW use type in Decl, and others
  Type *m_type = NULL;
  std::string m_name;
  XMLNode m_xmlnode;
  // TODO m_dimension
  // this is dimensions
  // if the code is char buf[5][4], the result is a vector of string "5" "4"
  // if the dimension is [], it will have an empty string, but the size of m_dims is still valid
  // Actually I only intend to use the size here.
  // A design decision:
  // whether to use a fixed length of buffer (of the stack), or use a variable length of buffer (on the heap)
  // std::vector<std::string> m_dims;
};

class DeclFactory {
public:
  static Decl* CreateDecl(XMLNode node);
};



// class Function;
// class Block;
// class Stmt;
// class Expr;
// class If;
// class Then;
// class Else;
// class ElseIf;
// class Switch;
// class Case;
// class Default;
// class While;
// class For;
// class Do;
// class ASTOther;

void ast_dfs(ASTNode *root, std::vector<ASTNode*> &visited);

class ASTNodeFactory {
public:
  static ASTNode* CreateASTNode(XMLNode xml_node, AST *ast, ASTNode *parent, ASTNode *prev);
};

class ASTNode {
public:
  typedef std::vector<ASTNode*>::iterator iterator;
  ASTNode(XMLNode xmlnode, AST *ast, ASTNode *parent, ASTNode *prev);
  virtual ~ASTNode() {}
  virtual ASTNodeKind Kind() = 0;
  /**
   * Default behavior: create under parent symbol table
   */
  virtual void CreateSymbolTable() {}
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
    
  // default  GetCondition return an empty node
  virtual XMLNode GetCondition() {return XMLNode();}
  virtual std::set<std::string> GetVarIds() {return {};}
  virtual std::set<std::string> GetIdToResolve() {return {};}


  virtual std::vector<Variable> GetVariables();

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
  ASTNode *GetFirstChild() {
    if (m_children.size() > 0) {
      return *m_children.begin();
    }
    return NULL;
  }
  ASTNode *GetLastChild() {
    if (m_children.size() > 0) {
      return m_children.back();
    }
    return NULL;
  }
  /**
   * All the children recursively
   */
  std::vector<ASTNode*> AllChildren() {
    std::vector<ASTNode*> ret;
    std::vector<ASTNode*> worklist;
    worklist.push_back(this);
    while (!worklist.empty()) {
      ASTNode *node = worklist.back();
      ret.push_back(node);
      worklist.pop_back();
      for (ASTNode *ch : node->Children()) {
        worklist.push_back(ch);
      }
    }
    return ret;
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
    return NULL;
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
  XMLNode GetXMLNode() {return m_xmlnode;}
  // should never set symbol table explicitly
  // void SetSymbolTable(SymbolTable *tbl) {m_sym_tbl = tbl;}
  SymbolTable *GetSymbolTable() {return m_sym_tbl;}
  int GetBeginLinum() {
    return get_node_line(m_xmlnode);
  }
  int GetEndLinum() {
    return get_node_last_line(m_xmlnode);
  }



  void SetFailurePoint() {
    if (m_ast) {
      m_ast->SetFailurePoint(this);
    }
  }
  void ClearFailurePoint() {
    if (m_ast) {
      m_ast->ClearFailurePoint(this);
    }
  }

  
protected:
  std::string POIOutputCode();
  std::string POIAfterCode();

  std::string FreedListCode();

    
  XMLNode m_xmlnode;
  AST *m_ast = NULL;
  ASTNode *m_parent = NULL;
  ASTNode *m_prev = NULL;
  
  std::vector<ASTNode*> m_children;
  // int m_index;
  SymbolTable *m_sym_tbl = NULL;
  // I'm not using these two fields because I need to initialize them in every Node class
  // Instead, the GetBeginLinum and GetEndLinum will compute them on-the-fly
  // this will of course bring some overhead
  // int m_begin_linum = 0;
  // int m_end_linum = 0;
};

#endif /* AST_NODE_H */
