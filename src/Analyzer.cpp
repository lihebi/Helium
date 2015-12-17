#include <Analyzer.hpp>
#include <iostream>
#include "util/ThreadUtil.hpp"
#include "Config.hpp"
#include "Logger.hpp"

Analyzer::Analyzer(const TestResult &test_result)
: m_test_result(test_result) {
  Logger::Instance()->LogTrace("[Analyzer::Analyzer]\n");
}

void
Analyzer::Analyze() {
  Logger::Instance()->LogTrace("[Analyzer::Analyze]\n");
  std::string cmd = std::string(std::getenv("HELIUM_HOME")) + "/scripts/compare.py -f " + Config::Instance()->GetOutputFolder()+"/out.csv";
  std::string result = ThreadUtil::Exec(cmd.c_str(), NULL);
  Logger::Instance()->LogData("analyze result:\n");
  Logger::Instance()->LogData(result+"\n");
}

Analyzer::~Analyzer() {}
