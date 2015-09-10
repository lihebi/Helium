#include <Tester.hpp>

Tester::Tester(const std::string &executable, const SegmentUnit &seg_unit, const Config &config)
: m_executable(executable), m_config(config) {
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
