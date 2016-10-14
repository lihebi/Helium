#include "analyzer.h"
#include "utils/utils.h"
#include "helium_options.h"
#include <iostream>

static const std::string TRANSFER_SCRIPT = "helium-analyze-transfer.R";
static const std::string META_SCRIPT = "helium-analyze-meta.R";

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
  std::string result_csv = m_dir + "/result.csv";
  std::string cmd;
  // transfer function
  cmd = TRANSFER_SCRIPT + " " + result_csv;
  // std::cout << "Analyzing using linear regression .." << "\n";
  // std::cout << cmd << "\n";
  m_transfer_output = utils::new_exec(cmd.c_str());

  cmd = META_SCRIPT + " " + result_csv;
  m_meta_output = utils::new_exec(cmd.c_str());

  
  // std::cout << "Transfer functions:" << "\n";
  if (HeliumOptions::Instance()->GetBool("print-analyze-result-transfer")) {
    std::cout << "Result Transfer Function: " << "\n";
    std::cout << m_transfer_output << "\n";
  }

  if (HeliumOptions::Instance()->GetBool("print-analyze-result-meta")) {
    std::cout << "Result Meta: " << "\n";
    std::cout << m_meta_output << "\n";
  }
}

bool Analyzer::IsCovered() {
  std::vector<std::string> lines = utils::split(m_meta_output, '\n');
  for (std::string line : lines) {
    if (line.find("Total reach poi") != std::string::npos) {
      int reached_count = std::stoi(utils::split(line, ':')[1]);
      if (reached_count > 0) return true;
      else return false;
    }
  }
  return false;
}

bool Analyzer::IsBugTriggered() {
  std::vector<std::string> lines = utils::split(m_meta_output, '\n');
  for (std::string line : lines) {
    if (line.find("Total fail poi") != std::string::npos) {
      int reached_count = std::stoi(utils::split(line, ':')[1]);
      if (reached_count > 0) return true;
      else return false;
    }
  }
  return false;
}
