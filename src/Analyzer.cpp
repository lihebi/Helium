#include <Analyzer.hpp>
#include <iostream>
#include "util/ThreadUtil.hpp"
#include "Config.hpp"

Analyzer::Analyzer(const TestResult &test_result)
: m_test_result(test_result) {
  std::cout << "[Analyzer::Analyzer]" << std::endl;
}

void
Analyzer::Analyze() {
  std::cout << "[Analyzer::Analyze]" << std::endl;
  std::string cmd = std::string(std::getenv("HELIUM_HOME")) + "/scripts/compare.py -f " + Config::Instance()->GetOutputFolder()+"/out.csv";
  std::string result = ThreadUtil::Exec(cmd);
  std::cout << "analyze result:" << std::endl;
  std::cout << result << std::endl;
}

Analyzer::~Analyzer() {}
