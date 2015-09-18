#include <Tester.hpp>

Tester::Tester(const std::string &executable, std::shared_ptr<SegmentProcessUnit> seg_unit)
: m_executable(executable) {
// generate input
// run program
// get output
}
Tester::~Tester() {}

bool Tester::Success() {
  return true;
}
TestResult Tester::GetOutput() {
  return m_test_result;
}
