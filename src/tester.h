#ifndef __TESTER_H__
#define __TESTER_H__

class Tester {
public:
  Tester (const std::string &executable, std::shared_ptr<SegmentProcessUnit> seg_unit);
  virtual ~Tester ();
  void Test();
  bool Success();
  TestResult GetOutput();
  void WriteCSV();

private:
  std::string generateInput();
  std::string m_executable;
  std::vector<Variable> m_inv;
  std::vector<Variable> m_outv;
  TestResult m_test_result;
  std::shared_ptr<SegmentProcessUnit> m_seg_unit;
  bool m_success;
};

#endif
