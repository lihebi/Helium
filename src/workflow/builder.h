#ifndef __BUILDER_H__
#define __BUILDER_H__
#include "segment.h"

class Builder {
public:
  Builder();
  virtual ~Builder();
  void SetMain(std::string main_code) {
    m_main = main_code;
  }
  void SetSupport(std::string support) {
    m_support = support;
  }
  void SetMakefile(std::string makefile) {
    m_makefile = makefile;
  }


  std::string GetMain() {
    return m_main;
  }
  std::string GetSupport() {
    return m_support;
  }
  std::string GetMakefile() {
    return m_makefile;
  }

  
  void AddScript(std::pair<std::string, std::string> script) {
    m_scripts.push_back(script);
  }
  void AddScript(std::string filename, std::string content) {
    m_scripts.push_back({filename, content});
  }
  void Write();
  void Compile();
  bool Success() {return m_success;}
  std::string GetExecutable();
  std::string GetDir() {return m_dir;}
  std::string GetExecutableName() {
    return "a.out";
  }
private:
  void preProcess();
  void postProcess();
  // void writeMain();
  // void writeSupport();
  // void writeMakefile();
  // Segment *m_seg;
  // Segment m_context;
  std::string m_main;
  std::string m_support;
  std::string m_makefile;
  bool m_success = false;
  std::string m_dir;
  std::vector<std::pair<std::string, std::string> > m_scripts;
};

#endif
