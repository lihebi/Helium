#ifndef ANALYZER_H
#define ANALYZER_H

#include "common.h"


class Analyzer {
public:
  Analyzer(std::string dir) : m_dir(dir) {}
  void GetCSV();
  void AnalyzeCSV();
  void ResolveQuery(std::string failure_condition);


  
  bool IsCovered();
  bool IsBugTriggered();
private:
  std::string m_dir;
  std::string m_transfer_output;
  std::string m_meta_output;
};

#endif /* ANALYZER_H */
