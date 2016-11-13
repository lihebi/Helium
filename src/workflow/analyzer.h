#ifndef ANALYZER_H
#define ANALYZER_H

#include "common.h"


class Analyzer {
public:
  Analyzer(std::string dir) : m_dir(dir) {}
  void GetCSV();
  void AnalyzeCSV();
  void ResolveQuery(std::string failure_condition);
  std::map<std::string, std::string> GetUsedTransfer() {
    return m_used_transfer;
  }

  bool IsCovered();
  bool IsBugTriggered();
private:
  std::string m_dir;
  std::string m_transfer_output;
  std::string m_meta_output;
  // this might not map to entry point
  // only valid if called after ResolveQuery!
  std::map<std::string, std::string> m_used_transfer;
};

#endif /* ANALYZER_H */
