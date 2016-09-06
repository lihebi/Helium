#ifndef CODE_TEST_H
#define CODE_TEST_H

#include "common.h"
#include "type/type.h"
#include "workflow/tester.h"

class CodeTester {
public:
  CodeTester(std::string exe_folder, std::string exe, std::map<std::string, Type*> inputs)
    : m_exe_folder(exe_folder), m_exe(exe), m_inputs(inputs) {}
  ~CodeTester() {}
  TestResult* Test();
  void Analyze(TestResult *result);
  
private:
  void genTestSuite();
  void freeTestSuite();
  std::string m_exe_folder;
  std::string m_exe;
  std::map<std::string, Type*> m_inputs;

  std::vector<std::vector<InputSpec*> > m_test_suite;
};

#endif /* CODE_TEST_H */
