#ifndef __SEGMENT_PROCESS_UNIT_HPP__
#define __SEGMENT_PROCESS_UNIT_HPP__

#include <iostream>
#include <vector>

#include "segment/Segment.hpp"
#include "Config.hpp"
#include "snippet/Snippet.hpp"
#include "Variable.hpp"

class SegmentProcessUnit {
public:
  SegmentProcessUnit();
  ~SegmentProcessUnit();
  // Reader functions
  void SetSegment(const Segment &s);
  void AddNode(pugi::xml_node);
  void Process();
  bool IncreaseContext();
  // builder functions
  std::string InstrumentIO();
  std::string GetContext();
  std::string GetMain();
  std::string GetSupport();
  std::string GetMakefile();

private:
  void contextSearch();
  void linearSearch(int value);

  void resolveInput();
  void resolveOutput();
  void resolveSnippets();

  // builder function
  std::string getInputCode();

  std::shared_ptr<Segment> m_segment;
  std::shared_ptr<Segment> m_context;
  std::set<Variable> m_inv;
  std::set<Variable> m_outv;
  std::set<Snippet*> m_snippets;

  std::vector<pugi::xml_node> m_functions;
  int m_linear_search_value;
};

#endif
