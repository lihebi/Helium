#ifndef ANALYZER_H
#define ANALYZER_H

#include "common.h"

class Analyzer {
public:
  Analyzer(std::string dir) : m_dir(dir) {}
  void GetCSV();
  void AnalyzeCSV();
  bool IsCovered();
private:
  std::string m_dir;
  std::string m_output;
};

#endif /* ANALYZER_H */
