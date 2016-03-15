#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include <pugixml.hpp>
#include <vector>
#include "type.h"

typedef enum {
  CSK_Linear,
  CSK_Slice
} ContextSearchKind;

#define BIT(x, i) (x >> i) & 1
#define SET_BIT(x, i) x |= (1 << i)
#define UNSET_BIT(x, i) x &= ~(1 << i)

typedef enum {
  DCK_Red,
  DCK_Green,
  DCK_Blue,
  DCK_Cyan,
  DCK_Black,
  DCK_Yellow,
  DCK_GreenYellow
} DotColorKind;

static const std::map<DotColorKind, std::string> DCK_MAP {
  {DCK_Red, "red"}
  , {DCK_Green, "green"}
  , {DCK_Blue, "blue"}
  , {DCK_Cyan, "cyan"}
  , {DCK_Black, "black"}
  , {DCK_Yellow, "yellow"}
  , {DCK_GreenYellow, "greenyellow"}
};
/**
 * 
 * 
 *          a
 *     b       c          l  m  n
 *   d   e        f
 * g    h i      j
 * 
 * width first search
 * 1
 * 5
 * 2,1,0,0,0
 * 1,2,1
 * 0,0,0,0

 * The string population sig is:
 * For every node, 0 or 1
 * x
 * xxxxx
 * xxx
 * xxxx
 * 
 * for example:
 * 1 00110 101 1101
 */

class ASTNode {
public:
  ASTNode() {}
  ~ASTNode() {}
  ASTNode *parent;
  // struct ASTNode *next_sibling;
  // struct ASTNode *first_child;
  std::vector<ASTNode*> children;
  int index;
  ast::Node value;
};

class AST {
public:
  // this root must be <function>
  AST(ast::Node root);
  ~AST() {
    for (ASTNode* n : m_local_nodes) {
      delete n;
    }
  }
  // void Load(ast::Node root);
  std::string GetSigStr();
  std::vector<int> GetSig();
  void Dump();
  int ComputeCommonParent(std::vector<int> indices);
  std::vector<int> ComputeRetreatingPath(int child, int parent);
  int ComputeDist(int child, int parent);
  void Visualize(std::map<int, DotColorKind> color_map=std::map<int, DotColorKind>(), std::string dir="random", bool display=false);
  std::string GetCode(std::set<int> indices = std::set<int>());
  std::string GetAllCode();
private:
  void load(ast::Node root, ASTNode *astnode);
  void reset() {
    m_sig.clear();
    m_nodes.clear();
  }
  // 0101010001110110001111
  std::vector<int> m_sig;
  // the sequence of all 0
  std::vector<ast::Node> m_nodes;
  std::vector<ASTNode*> m_local_nodes;
};

std::vector<int> rand_gene(size_t size);

/**
 * Population is based on AST.
 *  - able to convert to a string, which enables cross-over and mutation
 *  - able to map to the actual xml nodes, which enables get the souce code
 */
class Individual {
public:
  Individual() {}
  ~Individual() {}
  // each population must be bound to an AST tree
  void SetAST(AST *ast) {
    m_ast = ast;
    m_gene.clear();
    std::vector<int> sig = m_ast->GetSig();
    // +1 because the root
    m_size = std::count(sig.begin(), sig.end(), 0) + 1;
  }
  std::string GetASTSigStr() {
    if (m_ast == NULL) return "";
    return m_ast->GetSigStr();
  }
  // convert to the sig string
  std::vector<int> GetGene() {
    return m_gene;
  }
  std::string GetGeneStr() {
    std::string ret;
    for (int a : m_gene) {
      ret += std::to_string(a);
    }
    return ret;
  }
  std::string GetCGeneStr() {
    std::string ret;
    for (int a : m_cgene) {
      ret += std::to_string(a);
    }
    return ret;
  }
  // load the population from a string of {0,1}
  void SetGene(std::vector<int> gene);
  void SetGene(std::string gene);
  // when setting gene, ensure the size is the same as this size
  size_t size() {
    return m_size;
  }
  void Visualize(std::string dir="random", bool display=false);
  std::vector<int> ComputeCompleteGene();
private:
  AST *m_ast;
  size_t m_size;
  std::vector<int> m_gene;
  std::vector<int> m_cgene;
};


/**
 * A block of code that is of interest.
 *
 * It should be able to store the nodes in AST level representing the code.
 * Meanwhile, it should not keep the document object itself.
 * So the outside world should make sure the document is alive.
 * Otherwise the segment object is not valid.
 *
 * The class should also provide some functionality for searching of context.
 * In other word, it should store the context represented also as a list of AST level nodes.
 * It should provide some method to increase context based on different context search method.
 * The context search method is defined in a enumerator.
 *
 * Segment also needs to have storage for IO variables.
 * Input variables are all the variables that is used but not defined, or used, defined, but not initialized.
 * Output variables depends on the output point and the output type.
 * These two information should be stored in the segment.
 * For example, the output may be
 * 1) inside a loop, 2) outside a loop,
 * 3) at the beginning of the segment(for segment precondition),
 * 4) or at the end of the segment.
 * The output type may be: variable, loop count.
 * Generally speaking, output variables are variable that alive at the output program point.
 * It also needs to have multiple output points available.
 *
 * The segment should be able to resolve and locate the support code snippets.
 * The dirty work is done in another module: resolver
 * The segment needs to invoke that module, as well as store the pointers of dependent snippets.
 * 
 * Possible variable simplification strategy should be implemented.
 * Possible code simplification strategy should be implemented.
 */
class Segment {
  /**
   * Common practice of Segment:
   * Segment seg;
   * while (seg.IncreaseContext()) {
   *   // constructing
   *   seg.ResolveInput();
   *   seg.ResolveOutput();
   *   seg.ResolveSnippet();
   *   // building
   *   seg.GetMain();
   *   seg.GetSupport();
   *   seg.GetMakefile();
   *   // testing
   *   seg.GetInputVar();
   *   seg.GetOutputVar();
   * }
   */

public:
  Segment ();
  virtual ~Segment ();
  /* construct */
  void PushBack(ast::Node node);
  void PushBack(ast::NodeList nodes);
  void PushFront(ast::Node node);
  void PushFront(ast::NodeList nodes);
  void Clear();

  /*******************************
   ** context search
   *******************************/
  bool Grow();
  void IncreaseContext();
  
  /* Getter */
  /*******************************
   ** Meta info
   *******************************/
  ast::NodeList GetNodes() const;
  ast::Node GetFirstNode() const;
  // deprecated
  std::string GetText() const;
  std::string GetTextExceptComment();
  // use this instead
  std::string GetSegmentText() const;
  // this is not used when getting main.
  // the used one is getContext, which performs the remove of return stmt.
  std::string GetContextText() const;
  int GetLineNumber() const;
  bool HasNode(ast::Node node) const;
  bool IsValid();
  std::string GetInvalidReason() const {return m_invalid_reason;}
  void instrument();
  void uninstrument();

  /*******************************
   ** Code output
   *******************************/
  std::string GetMain();
  std::string GetSupport();
  std::string GetMakefile();
  std::vector<std::pair<std::string, std::string> > GetScripts();
  /*******************************
   ** IO Variables
   *******************************/
  void ResolveInput();
  void ResolveOutput();
  VariableList GetInputVariables() const {return m_inv;}
  VariableList GetOutputVariables() const {return m_outv;}

  /*******************************
   ** Resolving
   *******************************/
  void ResolveSnippets();

  /*******************************
   ** Code simplification
   *******************************/


   



private:
  std::string getHeader();
  std::string getInputCode();
  std::string getContext();
  // std::string m_filename;
  ast::NodeList m_nodes;
  ast::NodeList m_function_nodes;
  ast::NodeList m_call_nodes; // for instrumentation only. Add // @HeliumCallSite
  ast::NodeList m_context;

  /*******************************
   ** Variables
   *******************************/
  VariableList m_inv;
  VariableList m_outv;
  /*******************************
   ** Snippets
   *******************************/
  std::set<Snippet*> m_snippets;
  int m_context_search_time = 0;
  std::string m_invalid_reason;
  bool m_context_search_failed = false;
  std::vector<ast::Node> m_instruments;

};

typedef std::vector<Segment> SegmentList;

std::set<std::string>
get_to_resolve(
               ast::NodeList nodes,
               std::set<std::string> known_to_resolve,
               std::set<std::string> known_not_resolve
               );


// class SPU {
// public:
// SPU(const std::string& filename);
// ~SPU();
// // Reader functions
// void SetSegment(const Segment &s);
// void AddNode(ast::Node);
// void AddNodes(ast::NodeList);
// void Process();
// bool IncreaseContext();
// // builder functions
// bool IsValid();
// bool CanContinue() const {return m_can_continue;}


// // general info
// int GetLineNumber() const {return m_segment.GetLineNumber();}

// Segment GetSegment() const {return m_segment;}

// private:
// std::string getContext();
// void contextSearch();
// void linearSearch(int value);

// void resolveInput();
// void resolveOutput();
// void resolveSnippets();

// void instrument();
// void uninstrument();

// void simplifyCode();
// void unsimplifyCode();
// void doSimplifyCode(ast::Node node, ast::Node key);

// // builder function
// std::string getInputCode();

// std::string m_filename;

// Segment m_segment;
// Segment m_context;
// VariableList m_inv;
// VariableList m_outv;
// ast::Node m_output_node;
// std::set<Snippet*> m_snippets;

// std::vector<ast::Node> m_functions;
// int m_linear_search_value = 0;
// bool m_can_continue = true;
// std::vector<ast::Node> m_omit_nodes;
// };


#endif
