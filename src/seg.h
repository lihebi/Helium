#ifndef SEG_H
#define SEG_H

#include "ast_node.h"

/**
 * This is the new Segment class.
 * It is different from the class in segment.h, in the sense that, it contains multiple ASTs
 */

class Seg;
class Ctx;

/**
 * This class CREATE and hold multiple ASTs
 * There's only one Seg for each POI
 * Free the ASTs after use.
 */
class Seg {
public:
  Seg(ast::XMLNode node);
  ~Seg();
  void SetPOI() {}
  void NextContext();
  ast::ASTNode* GetFirstNode() {
    assert(!m_nodes.empty());
    return (*m_nodes.begin())->GetAST()->GetFirstNode(m_nodes);
  }
private:
  ast::AST* getAST(ast::XMLNode function_node);
  std::map<ast::XMLNode, ast::AST*> m_asts_m; // map from function xml node, to AST created.
  std::vector<Ctx*> m_ctxs;
  std::set<ast::ASTNode*> m_nodes; // POI
};

/**
 * The Context class.
 * This class must be associated with a Seg.
 * The nodes selection is the AST nodes, held by Seg.
 */
class Ctx {
public:
  ~Ctx() {}
  Ctx(Seg *seg) : m_seg(seg) {
    // this is the beginning.
    // use the POI as the first node.
    SetFirstNode(seg->GetFirstNode());
  }
  // copy constructor
  Ctx(const Ctx &rhs) {
    m_seg = rhs.m_seg;
    m_nodes = rhs.m_nodes;
  }
  Seg* GetSeg() {
    return m_seg;
  }
  ast::ASTNode *GetFirstNode() {
    return m_first;
  }
  void SetFirstNode(ast::ASTNode* node) {
    m_first = node;
  }

  /**
   * Test dynamic properties, to decide the first node to retain or not.
   * It is pretty weird here, because the old design is to test in reader.
   * This will incur the builder and tester, and backend.
   * The result of test:
   * 1. whether the first node will be in the selection of this or not
   * 2. the first node will not be affected by the result, because it decides where to search from for next context
   */
  void Test();
private:
  Seg *m_seg = NULL;
  std::set<ast::ASTNode*> m_nodes; // the selection of this context
  ast::ASTNode *m_first;
};
#endif /* SEG_H */
