#ifndef __BUILDER_H__
#define __BUILDER_H__
#include "segment.h"

class Builder {
public:
  Builder(SPU spu);
  virtual ~Builder();
  void Build();
  void Compile();
  bool Success() {return m_success;}
  std::string GetExecutable();
private:
  void writeMain();
  void writeSupport();
  void writeMakefile();
  SPU m_spu;
  // Segment m_context;
  std::string m_main;
  std::string m_support;
  std::string m_makefile;
  bool m_success;
};

#endif
