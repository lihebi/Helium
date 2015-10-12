#ifndef __ANALYZER_HPP__
#define __ANALYZER_HPP__

#include <TestResult.hpp>

class Analyzer {
public:
  Analyzer(const TestResult &test_result);
  virtual ~Analyzer();
  void Analyze();
private:
  TestResult m_test_result;
};

#endif
