#ifndef __TESTER_HPP__
#define __TESTER_HPP__

#include <string>
#include <vector>

#include "variable/Variable.hpp"
#include "Config.hpp"
#include "TestResult.hpp"
#include "segment/SegmentProcessUnit.hpp"

class Tester {
public:
  Tester (const std::string &executable, std::shared_ptr<SegmentProcessUnit> seg_unit);
  virtual ~Tester ();
  bool Success();
  TestResult GetOutput();

private:
  std::string m_executable;
  std::vector<Variable> m_inv;
  std::vector<Variable> m_outv;
  TestResult m_test_result;
};

#endif
