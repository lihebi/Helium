#include "analyzer.h"
#include "config.h"

#include <iostream>

Analyzer::Analyzer() {}

void
Analyzer::Analyze() {
  // std::string cmd = std::string(std::getenv("HELIUM_HOME")) + "/scripts/compare.py -f " + Config::Instance()->GetOutputFolder()+"/out.csv";
}

Analyzer::~Analyzer() {}
