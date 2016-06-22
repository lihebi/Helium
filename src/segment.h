#ifndef SEGMENT_H
#define SEGMENT_H

#include "ast_node.h"

std::string get_head();
std::string get_foot();
std::string get_header();

NewType* resolve_type(std::string var, ast::ASTNode *node);
std::string replace_return_to_35(const std::string &code);

/**
 * This is the new Segmentment class.
 * It is different from the class in segment.h, in the sense that, it contains multiple ASTs
 */

class Segment;
class Context;

/**
 * This class CREATE and hold multiple ASTs
 * There's only one Segment for each POI
 * Free the ASTs after use.
 */
class Segment {
public:
  Segment(ast::XMLNode node);
  ~Segment();
  void SetPOI() {}
  bool NextContext();
  void TestNextContext();
  bool ContinueNextContext() {
    if (m_resolved) return false;
    return !m_context_worklist.empty();
  }
  ast::ASTNode* GetFirstNode() {
    assert(!m_nodes.empty());
    return (*m_nodes.begin())->GetAST()->GetFirstNode(m_nodes);
  }
  std::set<ast::ASTNode*> GetPOI() {
    return m_nodes;
  }

  void DeclOutput() {
    m_poi_ast->SetOutput(m_output_vars);
  }
  void UnDeclOutput() {
    m_poi_ast->ClearOutput();
  }
private:
  // ast::AST* getAST(ast::XMLNode function_node);
  // std::map<ast::XMLNode, ast::AST*> m_asts_m; // map from function xml node, to AST created.
  ast::XMLDoc* createCallerDoc(ast::AST *ast);
  ast::XMLNode getCallerNode(ast::AST *ast);

  /**
   * Local storage. Free after use.
   */
  std::map<std::string, ast::AST*> m_func_to_ast_m; // map from function name to AST
  std::vector<ast::AST*> m_asts;
  ast::AST *m_poi_ast = NULL;
  std::vector<ast::XMLDoc*> m_docs;
  std::vector<Context*> m_ctxs;


  std::deque<Context*> m_context_worklist;
  
  // FIXME this should be a vector
  // but for now, it is ok, we only use one node
  std::set<ast::ASTNode*> m_nodes; // POI
  std::map<ast::ASTNode*, std::set<std::string> > m_deco; // POI output decoration of AST
  std::map<ast::ASTNode*, std::vector<NewVariable> > m_output_vars;

  bool m_resolved = false;
};


#endif /* SEGMENT_H */


