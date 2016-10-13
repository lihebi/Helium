#include "analyzer.h"
#include "utils/utils.h"
#include "helium_options.h"
#include <iostream>

void Analyzer::GetCSV() {
  // call the script helium-output-to-csv.py
  std::string cmd = "helium-output-to-csv.py " + m_dir + "/result.txt > " + m_dir + "/result.csv";
  utils::new_exec(cmd.c_str());
  std::cout << "generated CSV file" << "\n";

  // DEBUG print out the csv file
  cmd = "cat " + m_dir + "/result.csv";
  std::string output = utils::new_exec(cmd.c_str());
  if (HeliumOptions::Instance()->Has("verbose")) {
    std::cout << output << "\n";
  }
  // std::cout << output << "\n";


  
}

void Analyzer::AnalyzeCSV() {
  // call the R script to analyze the transfer function
  std::string cmd = "helium-transfer.R " + m_dir + "/result.csv";
  std::cout << "Analyzing using linear regression .." << "\n";
  std::cout << cmd << "\n";
  m_output = utils::new_exec(cmd.c_str());
  // std::cout << "Transfer functions:" << "\n";
  if (HeliumOptions::Instance()->GetBool("print-analyze-result")) {
    std::cout << "Analyze result: " << "\n";
    std::cout << m_output << "\n";
  }
}

bool Analyzer::IsCovered() {
  // TODO NOW
  std::vector<std::string> lines = utils::split(m_output, '\n');
  for (std::string line : lines) {
    if (line.find("Total reach poi") != std::string::npos) {
      int reached_count = std::stoi(utils::split(line, ':')[1]);
      if (reached_count > 0) return true;
      else return false;
    }
  }
  return false;
}
