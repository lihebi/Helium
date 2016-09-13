#ifndef CODE_TEST_H
#define CODE_TEST_H

#include "common.h"
#include "type/type.h"
#include "workflow/tester.h"

class TestSuite {
public:
  TestSuite() {}
  ~TestSuite() {}
  std::string GetInput() {
    std::string ret;
    for (auto &m : m_data) {
      ret += m.second->GetRaw() + "\n";
    }
    return ret;
  }
  std::string GetSpec() {
    std::string ret;
    for (auto &m : m_data) {
      ret += m.first + ": ";
      ret += m.second->GetSpec();
      ret += "\n";
    }
    return ret;
  }
  void Add(std::string var, InputSpec *spec) {
    m_data.push_back({var, spec});
  }
private:
  std::vector<std::pair<std::string, InputSpec*> > m_data;
};

class CodeTester {
public:
  CodeTester(std::string exe_folder, std::string exe, std::map<std::string, Type*> inputs)
    : m_exe_folder(exe_folder), m_exe(exe), m_inputs(inputs) {}
  ~CodeTester() {}
  void Test();
  void Analyze(TestResult *result);
  
private:
  void genTestSuite();
  void freeTestSuite();
  std::string m_exe_folder;
  std::string m_exe;
  std::map<std::string, Type*> m_inputs;

  std::vector<TestSuite> m_test_suites;

  std::set<InputSpec*> m_specs;
};

#endif /* CODE_TEST_H */
