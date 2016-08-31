#ifndef CFG_H
#define CFG_H

#include "ast.h"
#include "ast_node.h"

class CFG;

class CFGNode {
public:
  CFGNode(CFG *cfg, ASTNode *astnode) : m_cfg(cfg), m_astnode(astnode) {}
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
  CFG *GetCFG() {return m_cfg;}
private:
  CFG *m_cfg = NULL;
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

  // CFGNode *GetCFGNode(ASTNode *astnode);
  // std::set<CFGNode*> GetPredecessors(CFGNode *node);


  /**
   * get predecessor cfg nodes
   */
  std::set<CFGNode*> GetPredecessors(CFGNode *node);
  CFGNode *ASTNodeToCFGNode(ASTNode *astnode) {
    // check if all m_nodes are in m_ast_to_cfg mapping
    // FIXME performance
    for (CFGNode *cfgnode : m_nodes) {
      if (m_cfg_to_ast.count(cfgnode) == 0) {
        m_cfg_to_ast[cfgnode] = cfgnode->GetASTNode();
        m_ast_to_cfg[cfgnode->GetASTNode()] = cfgnode;
      }
    }
    // get it
    if (m_ast_to_cfg.count(astnode) == 1) {
      return m_ast_to_cfg[astnode];
    }
    return NULL;
  }
  
  void Visualize(std::set<CFGNode*> nodesA = {}, std::set<CFGNode*> nodesB = {}, bool open=true);

  void AddBreak(CFGNode *node) {m_breaks.insert(node);}
  void AddContinue(CFGNode *node) {m_continues.insert(node);}
  void AddReturn(CFGNode *node) {m_returns.insert(node);}

  void AdjustBreak();
  void AdjustContinue();
  void AdjustReturn();

  bool Contains(CFGNode* cfgnode) {
    return m_nodes.count(cfgnode) == 1;
  }
private:
  void copyEdge(CFG *cfg);
  std::set<CFGNode*> m_nodes;
  // std::map<ASTNode*,CFGNode*> m_ast2cfg;
  // CFGNode *m_root = NULL;
  std::set<CFGNode*> m_ins;
  std::set<CFGNode*> m_outs;

  std::set<CFGNode*> m_breaks;
  std::set<CFGNode*> m_continues;
  std::set<CFGNode*> m_returns;

  std::map<ASTNode*, CFGNode*> m_ast_to_cfg;
  std::map<CFGNode*, ASTNode*> m_cfg_to_ast;
  
  std::map<CFGNode*, std::set<CFGNode*> > m_edges;
  std::map<std::pair<CFGNode*, CFGNode*>, std::string> m_labels;

  std::map<CFGNode*, std::set<CFGNode*> > m_back_edges;

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
