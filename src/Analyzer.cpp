#include <Analyzer.hpp>
#include <iostream>
#include "util/ThreadUtil.hpp"
#include "Config.hpp"

Analyzer::Analyzer(const TestResult &test_result)
: m_test_result(test_result) {
}

void
Analyzer::Analyze() {
  std::string cmd = std::string(std::getenv("HELIUM_HOME")) + "/scripts/compare.py -f " + Config::Instance()->GetOutputFolder()+"/out.csv";
  std::string result = ThreadUtil::Exec(cmd.c_str(), NULL);
}

Analyzer::~Analyzer() {}
