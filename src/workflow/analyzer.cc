#include "analyzer.h"
#include "utils/utils.h"

void Analyzer::GetCSV() {
  // call the script helium-output-to-csv.py
  std::string cmd = "helium-output-to-csv.py " + m_dir + "/result.txt > " + m_dir + "/result.csv";
  utils::new_exec(cmd.c_str());
}

void Analyzer::AnalyzeCSV() {
  // call the R script to analyze the transfer function
}
