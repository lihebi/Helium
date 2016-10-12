#ifndef SEGMENT_H
#define SEGMENT_H

#include "common.h"
#include "parser/ast.h"
#include "parser/cfg.h"

class Selection {
};


class Segment {
public:
  Segment() {}
  ~Segment() {}
  Segment(const Segment &q) {
    m_nodes = q.m_nodes;
    m_new = q.m_new;
  }
  Segment(ASTNode *astnode);
  Segment(CFGNode *cfgnode) {
    // TODO ensure this is not already in m_nodes, otherwise the m_new will not be ideal
    m_nodes.insert(cfgnode);
    m_new = cfgnode;
  }
  void Merge(Segment *q) {
    std::set<CFGNode*> nodes = q->GetNodes();
    m_nodes.insert(nodes.begin(), nodes.end());
  }
  void Add(CFGNode *node, bool inter=false) {
    assert(node);
    m_nodes.insert(node);
    m_new = node;
    m_inter = inter;
  }

  bool IsInter() {
    return m_inter;
  }

  /**
   * Remove a node.
   * This node is typically marked as "bad".
   * This node is typically m_new;
   * But I will not change m_new to something else, because I need to do context search from there.
   */
  void Remove(CFGNode *node) {
    if (m_nodes.count(node) == 1) {
      m_nodes.erase(node);
    }
  }
  CFGNode* New() {
    return m_new;
  }
  std::set<CFGNode*> GetNodes() {
    return m_nodes;
  }

  // TODO nodes in the CFG that contains m_new
  std::set<CFGNode*> GetNodesForNewFunction();
  bool ContainNode(CFGNode *node) {
    if (m_nodes.count(node) == 1) return true;
    else return false;
  }

  void Visualize(bool open=true);


  /**
   * Code generating
   */
  void ResolveInput();
  void GenCode();
  std::string GetMain() {return m_main;}
  std::string GetSupport() {return m_support;}
  std::string GetMakefile() {return m_makefile;}
  
  std::map<std::string, Type*> GetInputs() {
    return m_inputs;
  }

  static void MarkBad(CFGNode *node) {
    m_bad.insert(node);
  }
  static bool IsBad(CFGNode *node) {
    if (m_bad.count(node) == 1) return true;
    return false;
  }


  // void GenTestSuite();
  // void Test();
private:
  std::set<CFGNode*> m_nodes;
  CFGNode *m_new = NULL;
  bool m_inter = false; // whether the new node is the result from inter-procedure or not
  // std::map<AST*, std::vector<Variable> > m_inputs;
  std::map<std::string, Type*> m_inputs;

  std::string m_main;
  std::string m_support;
  std::string m_makefile;

  static std::set<CFGNode*> m_bad;

  // FIXME should not be copied when the query propagate?
  std::string m_opt;
};


#endif /* SEGMENT_H */
