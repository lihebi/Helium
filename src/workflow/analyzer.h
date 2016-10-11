#ifndef ANALYZER_H
#define ANALYZER_H

#include "common.h"

class Analyzer {
public:
  Analyzer(std::string dir) : m_dir(dir) {}
  void GetCSV();
  void AnalyzeCSV();
private:
  std::string m_dir;
};

#endif /* ANALYZER_H */
