#ifndef SEGMENT_H
#define SEGMENT_H

#include "parser/ast_node.h"
#include "type/variable.h"

std::string get_head();
std::string get_foot();
std::string get_header();

Type* resolve_type(std::string var, ASTNode *node);
std::string replace_return_to_35(const std::string &code);

/**
 * This is the new Segmentment class.
 * It is different from the class in segment.h, in the sense that, it contains multiple ASTs
 */

class Segment;
class Context;

/**
 * Segment kind can be: stmt, continuous stmts, un-continuous stmts, loop
 */
typedef enum _SegmentKind {
  SegKind_Stmt,
  SegKind_Loop
} SegmentKind;

/**
 * This class CREATE and hold multiple ASTs
 * There's only one Segment for each POI
 * Free the ASTs after use.
 */
class Segment {
public:
  Segment(XMLNode node, SegmentKind kind = SegKind_Stmt);
  ~Segment();
  void SetPOI() {}
  bool NextContext();
  void TestNextContext();
  bool ContinueNextContext() {
    if (m_resolved) return false;
    return !m_context_worklist.empty();
  }
  ASTNode* GetFirstNode() {
    assert(!m_nodes.empty());
    return (*m_nodes.begin())->GetAST()->GetFirstNode(m_nodes);
  }
  std::set<ASTNode*> GetPOI() {
    return m_nodes;
  }

  void DeclOutput() {
    m_poi_ast->SetOutput(m_output_vars);
  }
  void UnDeclOutput() {
    m_poi_ast->ClearOutput();
  }

  std::set<std::string> GetConditions() {
    return m_jump_out_cond;
  }
private:
  // AST* getAST(XMLNode function_node);
  // std::map<XMLNode, AST*> m_asts_m; // map from function xml node, to AST created.
  XMLDoc* createCallerDoc(AST *ast);
  XMLNode getCallerNode(AST *ast);

  // dis-assembling the constructor
  void getMetaData();
  void createAST();
  void createPOI();
  void createOutputVars();

  void extractJumpOutCondition();

  XMLNode m_xmlnode;
  SegmentKind m_segkind;
  std::string m_func_name; // the function contains the segment
  XMLNode m_function_node; // the function node in XML format

  // loop related

  // conditions inside the loop (including the loop condition itself)
  std::set<std::string> m_jump_out_cond;
  /**
   * Local storage. Free after use.
   */
  std::map<std::string, AST*> m_func_to_ast_m; // map from function name to AST
  std::vector<AST*> m_asts;
  AST *m_poi_ast = NULL; // AST that contains POI
  std::vector<XMLDoc*> m_docs;
  std::vector<Context*> m_ctxs;


  std::deque<Context*> m_context_worklist;
  
  // FIXME this should be a vector
  // but for now, it is ok, we only use one node

  // This is the set of nodes initially in the segment
  std::set<ASTNode*> m_nodes;
  // this is the POI position.
  // POI should be only one for any segment selected, at least for now.
  // The position is actually right before this node.
  // It can be the situation that the POI is after all the segment, so the m_poi might be out of m_nodes
  // However, if the POI is going to be AFTER the last node, this code cannot handle it.
  // The "last node" problem might happen in loop situaion.
  // Other cases should not have this problem because we usually do not care about the status after the last node.
  ASTNode *m_poi;
  
  std::map<ASTNode*, std::vector<Variable> > m_output_vars;

  bool m_resolved = false;
};


#endif /* SEGMENT_H */


