#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include <pugixml.hpp>
#include <vector>
#include "type.h"

class Segment {
public:
  Segment ();
  virtual ~Segment ();
  /* construct */
  void PushBack(ast::Node node);
  void PushBack(ast::NodeList nodes);
  void PushFront(ast::Node node);
  void PushFront(ast::NodeList nodes);
  void Clear();
  void Print();

  /* grow */
  void Grow();
  
  /* Getter */
  ast::NodeList GetNodes() const;
  ast::Node GetFirstNode() const;
  /* text */
  std::string GetText();
  std::string GetTextExceptComment();
  int GetLineNumber() const;
  int GetLOC() const {return m_loc;}
  /* attr */
  bool HasNode(ast::Node node) const;
  bool IsValid() const {return m_valid;}
private:
  void updateMeta();
  ast::NodeList m_nodes;
  ast::NodeList m_function_nodes;
  int m_loc = 0;
  bool m_valid = false;
};

typedef std::vector<Segment> SegmentList;

class SPU {
public:
  SPU(const std::string& filename);
  ~SPU();
  // Reader functions
  void SetSegment(const Segment &s);
  void AddNode(ast::Node);
  void AddNodes(ast::NodeList);
  void Process();
  bool IncreaseContext();
  // builder functions
  std::string GetMain();
  std::string GetSupport();
  std::string GetMakefile();
  bool IsValid();
  bool CanContinue() const {return m_can_continue;}

  VariableList GetInputVariables() const {return m_inv;}
  VariableList GetOutputVariables() const {return m_outv;}

  // general info
  const std::string& GetFilename() const {return m_filename;}
  int GetLineNumber() const {return m_segment.GetLineNumber();}

  Segment GetSegment() const {return m_segment;}

private:
  std::string getContext();
  void contextSearch();
  void linearSearch(int value);

  void resolveInput();
  void resolveOutput();
  void resolveSnippets();

  void instrument();
  void uninstrument();

  void simplifyCode();
  void unsimplifyCode();
  void doSimplifyCode(ast::Node node, ast::Node key);

  // builder function
  std::string getInputCode();

  std::string m_filename;

  Segment m_segment;
  Segment m_context;
  VariableList m_inv;
  VariableList m_outv;
  ast::Node m_output_node;
  std::set<Snippet*> m_snippets;

  std::vector<ast::Node> m_functions;
  int m_linear_search_value = 0;
  bool m_can_continue = true;
  std::vector<ast::Node> m_omit_nodes;
};


#endif
