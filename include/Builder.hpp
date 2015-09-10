#ifndef __BUILDER_HPP__
#define __BUILDER_HPP__

#include <Config.hpp>
#include <vector>
#include <IOVariable.hpp>
#include <Segment.hpp>
#include <Support.hpp>
#include "SegmentUnit.hpp"

class Builder {
public:
  Builder(const SegmentUnit &seg_unit, const Config &config);
  virtual ~Builder();
  bool Success();
  std::string GetExecutable();
private:
  std::vector<IOVariable> m_inv;
  std::vector<IOVariable> m_outv;
  Segment m_context;
  Support m_support;
  Config m_config;
  std::string m_executable;
};

#endif
