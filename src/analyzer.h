#ifndef __ANALYZER_H__
#define __ANALYZER_H__

class Analyzer {
public:
  Analyzer(const TestResult &test_result);
  virtual ~Analyzer();
  void Analyze();
private:
  TestResult m_test_result;
};


#endif
