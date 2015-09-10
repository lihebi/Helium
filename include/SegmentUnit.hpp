#ifndef __SEGMENT_UNIT_HPP__
#define __SEGMENT_UNIT_HPP__

#include <iostream>
#include <vector>

#include "Segment.hpp"
#include "Support.hpp"
#include "IOVariable.hpp"
#include "Config.hpp"

class SegmentUnit {
public:
  SegmentUnit(const Config &config);
  ~SegmentUnit();
  void SetSegment(const Segment &s);
  void AddSegmentNode(pugi::xml_node);
  void Process();

private:
  void contextSearch();
  void linearSearch(int value);
  bool increaseContext(int value);

  void resolveInput();
  void resolveOutput();
  void resolveSupport();

  std::shared_ptr<Segment> m_segment;
  std::shared_ptr<Segment> m_context;
  std::shared_ptr<Support> m_support;
  std::vector<IOVariable> m_inv;
  std::vector<IOVariable> m_outv;

  std::vector<pugi::xml_node> m_functions;

  Config m_config;
};

#endif
