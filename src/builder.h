#ifndef __BUILDER_H__
#define __BUILDER_H__
#include "segment.h"

class Builder {
public:
  Builder(Segment* seg);
  virtual ~Builder();
  void Write();
  void Compile();
  bool Success() {return m_success;}
  std::string GetExecutable();
  std::string GetDir() {return m_dir;}
private:
  void writeMain();
  void writeSupport();
  void writeMakefile();
  Segment *m_seg;
  // Segment m_context;
  std::string m_main;
  std::string m_support;
  std::string m_makefile;
  bool m_success;
  std::string m_dir;
};

#endif
