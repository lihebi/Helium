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
  bool NextContext();
  ast::ASTNode* GetFirstNode() {
    assert(!m_nodes.empty());
    return (*m_nodes.begin())->GetAST()->GetFirstNode(m_nodes);
  }
  std::set<ast::ASTNode*> GetPOI() {
    return m_nodes;
  }
private:
  // ast::AST* getAST(ast::XMLNode function_node);
  // std::map<ast::XMLNode, ast::AST*> m_asts_m; // map from function xml node, to AST created.
  ast::XMLDoc* createCallerDoc(ast::AST *ast);

  /**
   * Local storage. Free after use.
   */
  std::map<std::string, ast::AST*> m_func_to_ast_m; // map from function name to AST
  std::vector<ast::AST*> m_asts;
  std::vector<ast::XMLDoc*> m_docs;
  std::vector<Ctx*> m_ctxs;
  std::set<ast::ASTNode*> m_nodes; // POI
  std::map<ast::ASTNode*, std::set<std::string> > m_deco; // POI output decoration of AST
};

/**
 * The Context class.
 * This class must be associated with a Seg.
 * The nodes selection is the AST nodes, held by Seg.
 */
class Ctx {
public:
  ~Ctx() {}
  Ctx(Seg *seg);
  // copy constructor
  Ctx(const Ctx &rhs);
  Seg* GetSeg() {
    return m_seg;
  }
  ast::ASTNode *GetFirstNode() {
    return m_first;
  }
  void SetFirstNode(ast::ASTNode* node);
  void AddNode(ast::ASTNode* node);
  void RemoveNode(ast::ASTNode *node);
  std::set<ast::ASTNode*> GetNodes() {
    return m_nodes;
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
  void dump();


  /**
   * The resolving, code output related methods
   * 1. resolve input
   * 2. resolve snippet
   * For each of the AST!
   */
  void Resolve();
  
private:
  /**
   * The input shoudl be associated with AST the first node belongs.
   * All other ASTs should not contain any input code?
   * Need to instrument the POI
   * No need to instrument input, because that's our generated inputs.
   */
  /**
   * Tasks:
   * 1. find the def, recursively, and include
   * 2. find the input variables, their type
   */
  std::set<ast::ASTNode*> resolveDecl(ast::AST *ast, bool to_root);
  /**
   * No magic, the old one suffices.
   */
  void resolveSnippet(ast::AST *ast);
  /**
   * Code getting
   */
  std::string getMain();
  std::string getSupport();
  std::string getMakefile();
  Seg *m_seg = NULL;
  /**
   * Storage
   */
  std::set<ast::ASTNode*> m_nodes; // the selection of this context
  std::map<ast::AST*, std::set<ast::ASTNode*> > m_ast_to_node_m;
  ast::ASTNode *m_first;
  // decoration of declaration and input on AST
  typedef std::map<ast::ASTNode*, std::set<std::string> > decl_deco;
  // local storage for the decoration of each AST
  std::map<ast::AST*, std::pair<decl_deco, decl_deco> > m_ast_to_deco_m;

  // this is another system for input code generation
  // this does not decorate the AST
  // instead, get the variables out, and insert at the beginning
  // typedef std::vector<std::pair<NewType*, std::string> > InputMetrics;

  /**
   * This is a map from variable name to its type
   */
  typedef std::map<std::string, NewType*> InputMetrics;
  // the following two can overlap, i.e. need both declaration and input
  std::map<ast::AST*, InputMetrics> m_decls; // only need declaraion
  std::map<ast::AST*, InputMetrics> m_inputs; // only need input
  
  std::set<int> m_snippet_ids; // only need one copy of snippet ids, for all the ASTs
};
#endif /* SEG_H */
