#ifndef __TESTER_H__
#define __TESTER_H__
#include "common.h"
#include "segment.h"

class Tester {
public:
  Tester (const std::string &executable, SPU spu);
  virtual ~Tester ();
  void Test();
  bool Success();
  // TestResult GetOutput();
  void WriteCSV();

private:
  std::string generateInput();
  std::string m_executable;
  std::vector<Variable> m_inv;
  std::vector<Variable> m_outv;
  // TestResult m_test_result;
  SPU m_spu;
  bool m_success;
};

#endif
