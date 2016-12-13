#ifndef ANALYZER_H
#define ANALYZER_H

#include "common.h"


class Analyzer {
public:
  Analyzer(std::string dir) : m_dir(dir) {}
  void GetCSV();
  void AnalyzeCSV();
  bool ResolveQuery(std::string failure_condition);
  bool ResolveQuery2(std::string failure_condition);
  std::map<std::string, std::string> GetUsedTransfer() {
    return m_used_transfer;
  }

  bool IsCovered();
  bool IsBugTriggered();
  /**
   * Whether the two analyzer using same set of transfer function
   */
  static bool same_trans(Analyzer *p1, Analyzer *p2);
  static void print_used_trans(Analyzer *p);
private:
  bool checkSat(std::vector<std::string> v, std::vector<std::string> vneg={});
  bool checkFc(std::vector<std::string> cons, std::string fc);
  bool checkNegfc(std::vector<std::string> cons, std::string fc);
  std::string m_dir;
  std::string m_transfer_output;
  std::string m_meta_output;
  // this might not map to entry point
  // only valid if called after ResolveQuery!
  std::map<std::string, std::string> m_used_transfer;
};

#endif /* ANALYZER_H */
