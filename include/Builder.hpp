#ifndef __BUILDER_HPP__
#define __BUILDER_HPP__

#include <Config.hpp>
#include <vector>
#include "segment/Segment.hpp"
#include "segment/SegmentProcessUnit.hpp"

class Builder {
public:
  Builder(std::shared_ptr<SegmentProcessUnit> seg_unit);
  virtual ~Builder();
  void Build();
  void Compile();
  bool Success();
  std::string GetExecutable();
private:
  void writeMain();
  void writeSupport();
  void writeMakefile();
  std::shared_ptr<SegmentProcessUnit> m_seg_unit;
  // Segment m_context;
  std::string m_executable;
  std::string m_main;
  std::string m_support;
  std::string m_makefile;
};

#endif
