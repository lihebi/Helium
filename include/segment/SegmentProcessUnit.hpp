#ifndef __SEGMENT_PROCESS_UNIT_HPP__
#define __SEGMENT_PROCESS_UNIT_HPP__

#include <iostream>
#include <vector>
#include <memory>

#include "segment/Segment.hpp"
#include "Config.hpp"
#include "snippet/Snippet.hpp"
#include "variable/Variable.hpp"

class SegmentProcessUnit {
public:
  SegmentProcessUnit(const std::string& filename);
  ~SegmentProcessUnit();
  // Reader functions
  void SetSegment(const Segment &s);
  void AddNode(pugi::xml_node);
  void Process();
  bool IncreaseContext();
  // builder functions
  std::string GetMain();
  std::string GetSupport();
  std::string GetMakefile();
  bool IsValid();

  std::set<std::shared_ptr<Variable> > GetInputVariables() const {return m_inv;}
  std::set<std::shared_ptr<Variable> > GetOutputVariables() const {return m_outv;}

  // general info
  const std::string& GetFilename() const {return m_filename;}
  int GetLineNumber() const {return m_segment->GetLineNumber();}

private:
  std::string getContext();
  void contextSearch();
  void linearSearch(int value);

  void resolveInput();
  void resolveOutput();
  void resolveSnippets();

  void instrument();
  void uninstrument();

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
};

#endif
