#include "analyzer.h"
#include "utils/utils.h"
#include <iostream>

void Analyzer::GetCSV() {
  // call the script helium-output-to-csv.py
  std::string cmd = "helium-output-to-csv.py " + m_dir + "/result.txt > " + m_dir + "/result.csv";
  utils::new_exec(cmd.c_str());


  // DEBUG print out the csv file
  cmd = "cat " + m_dir + "/result.csv";
  std::string output = utils::new_exec(cmd.c_str());
  // std::cout << output << "\n";


  
}

void Analyzer::AnalyzeCSV() {
  // call the R script to analyze the transfer function
  std::string cmd = "helium-transfer.R " + m_dir + "/result.csv";
  std::string output = utils::new_exec(cmd.c_str());
  std::cout << "Transfer functions:" << "\n";
  std::cout << output << "\n";
}
