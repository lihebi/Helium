#ifndef CFG_H
#define CFG_H

#include "ast.h"

class CFGNode {
public:
  CFGNode(ASTNode *astnode) : m_astnode(astnode) {}
  ~CFGNode() {}
  ASTNode *GetASTNode() {return m_astnode;}
  void AddPredecessor(CFGNode *node) {
    if (node) {
      m_predecessors.insert(node);
    }
  }
  void AddSuccessor(CFGNode *node) {
    if (node) {
      m_successors.insert(node);
    }
  }
private:
  ASTNode *m_astnode;
  std::set<CFGNode*> m_predecessors;
  std::set<CFGNode*> m_successors;
};

class CFG {
public:
  CFG(AST *ast);
  ~CFG();
  CFGNode *GetCFGNode(ASTNode* astnode);
  void CreateCFGNode(ASTNode *astnode);
  void CreateEdge(CFGNode* from, CFGNode *to);
private:
  void traverse(CFGNode *node);
  AST *m_ast = NULL;
  std::vector<CFGNode*> m_nodes;
  std::map<ASTNode*,CFGNode*> m_ast2cfg;
  CFGNode *m_root = NULL;
};


class CFGFactory {
public:
  static CFG *CreateCFG(ASTNode *node);
  static CFG *CreateCFGFromIf(ASTN)
};


#endif /* CFG_H */
