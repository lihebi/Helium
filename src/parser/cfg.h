#ifndef CFG_H
#define CFG_H

#include "ast.h"
#include "ast_node.h"

class CFGNode {
public:
  CFGNode(ASTNode *astnode) : m_astnode(astnode) {}
  ~CFGNode() {}
  ASTNode *GetASTNode() {return m_astnode;}
  
  
  std::string GetID() {
    const void * address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;  
    std::string ret = ss.str();
    ret = "\"" + ret + "\"";
    return ret;
  }
  std::string GetLabel() {
    if (m_astnode) {
      return m_astnode->GetLabel();
    }
    return "";
  }
  // void AddPredecessor(CFGNode *node) {
  //   if (node) {
  //     m_predecessors.insert(node);
  //   }
  // }
  // void AddSuccessor(CFGNode *node) {
  //   if (node) {
  //     m_successors.insert(node);
  //   }
  // }
private:
  ASTNode *m_astnode = NULL;
  // std::set<CFGNode*> m_predecessors;
  // std::set<CFGNode*> m_successors;
};

/**
 * TODO free so many CFGs
 * TODO break, continue, return
 */
class CFG {
public:
  CFG();
  ~CFG();

  void AddNode(CFGNode *node);
  void Merge(CFG* cfg);
  void MergeBranch(CFG *cfg, bool b);
  void MergeCase(CFG *cfg);

  int GetBranchNum() {return m_branch_num;}
  void RemoveOut(CFGNode *node) {
    if (m_outs.count(node) == 1) {
      m_outs.erase(node);
    }
  }

  /**
   * Set the condition node. Used to merge branch.
   */
  void SetCond(CFGNode *node) {m_cond = node;}

  void CreateEdge(CFGNode *from, CFGNode *to, std::string label="");

  void AddIn(CFGNode *node) {
    if (node) m_ins.insert(node);
  }
  void AddOut(CFGNode *node) {
    if (node) m_outs.insert(node);
  }
  std::set<CFGNode*> GetIns() {return m_ins;}
  std::set<CFGNode*> GetOuts() {return m_outs;}
  std::set<CFGNode*> GetNodes() {return m_nodes;}
  
  void Visualize();
private:
  void copyEdge(CFG *cfg);
  std::set<CFGNode*> m_nodes;
  // std::map<ASTNode*,CFGNode*> m_ast2cfg;
  // CFGNode *m_root = NULL;
  std::set<CFGNode*> m_ins;
  std::set<CFGNode*> m_outs;
  
  std::map<CFGNode*, std::set<CFGNode*> > m_edges;
  std::map<std::pair<CFGNode*, CFGNode*>, std::string> m_labels;

  // sub graph utility
  CFGNode *m_cond = NULL;
  int m_branch_num = 0;
};


class CFGFactory {
public:
  static CFG *CreateCFG(AST *ast);
  static CFG *CreateCFG(ASTNode *node);
  static CFG *CreateCFGFromIf(If *node);
  static CFG *CreateCFGFromFunction(Function *node);
  static CFG *CreateCFGFromElseIf(ElseIf *node);
  static CFG *CreateCFGFromSwitch(Switch *node);
  static CFG *CreateCFGFromWhile(While *node);
  static CFG *CreateCFGFromFor(For *node);
  static CFG *CreateCFGFromDo(Do *node);
  static CFG *CreateCFGFromBlock(Block *block);
};


#endif /* CFG_H */
