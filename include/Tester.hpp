#ifndef __TESTER_HPP__
#define __TESTER_HPP__

#include <string>
#include <vector>

#include <IOVariable.hpp>
#include <Config.hpp>
#include <TestResult.hpp>
#include "SegmentUnit.hpp"

class Tester {
public:
  Tester (const std::string &executable, const SegmentUnit &seg_unit, const Config &config);
  virtual ~Tester ();
  bool Success();
  TestResult GetOutput();

private:
  std::string m_executable;
  std::vector<IOVariable> m_inv;
  std::vector<IOVariable> m_outv;
  Config m_config;
  TestResult m_test_result;
};

#endif
