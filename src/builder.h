#ifndef __BUILDER_H__
#define __BUILDER_H__
#include "segment.h"

class Builder {
public:
  Builder(Segment seg);
  virtual ~Builder();
  void Build();
  void Compile();
  bool Success() {return m_success;}
  std::string GetExecutable();
private:
  void writeMain();
  void writeSupport();
  void writeMakefile();
  Segment m_seg;
  // Segment m_context;
  std::string m_main;
  std::string m_support;
  std::string m_makefile;
  bool m_success;
};

#endif
