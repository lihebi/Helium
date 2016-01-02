#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include <pugixml.hpp>
#include <vector>
#include "variable.h"

class Segment {
public:
  Segment ();
  virtual ~Segment ();
  void PushBack(pugi::xml_node node);
  void PushFront(pugi::xml_node node);
  void Clear();
  void Print();
  std::vector<pugi::xml_node> GetNodes() const;
  pugi::xml_node GetFirstNode() const;
  std::string GetText();
  std::string GetTextExceptComment();
  int GetLineNumber() const;
  int GetLOC() const {return m_loc;}
  bool HasNode(pugi::xml_node node) const;
private:
  std::vector<pugi::xml_node> m_nodes;
  int m_loc = 0;
};

class SPU {
public:
  SPU(const std::string& filename);
  ~SPU();
  // Reader functions
  void SetSegment(const Segment &s);
  void AddNode(pugi::xml_node);
  void AddNodes(std::vector<pugi::xml_node>);
  void Process();
  bool IncreaseContext();
  // builder functions
  std::string GetMain();
  std::string GetSupport();
  std::string GetMakefile();
  bool IsValid();
  bool CanContinue() const {return m_can_continue;}

  std::set<std::shared_ptr<Variable> > GetInputVariables() const {return m_inv;}
  std::set<std::shared_ptr<Variable> > GetOutputVariables() const {return m_outv;}

  // general info
  const std::string& GetFilename() const {return m_filename;}
  int GetLineNumber() const {return m_segment->GetLineNumber();}

  std::shared_ptr<Segment> GetSegment() const {return m_segment;}

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
  void doSimplifyCode(pugi::xml_node node, pugi::xml_node key);

  // builder function
  std::string getInputCode();

  std::string m_filename;

  std::shared_ptr<Segment> m_segment;
  std::shared_ptr<Segment> m_context;
  std::set<std::shared_ptr<Variable> > m_inv;
  std::set<std::shared_ptr<Variable> > m_outv;
  pugi::xml_node m_output_node;
  std::set<Snippet*> m_snippets;

  std::vector<pugi::xml_node> m_functions;
  int m_linear_search_value;
  bool m_can_continue = true;
  std::vector<pugi::xml_node> m_omit_nodes;
};


#endif
