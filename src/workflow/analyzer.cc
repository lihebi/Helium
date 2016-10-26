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

/**
 * Whether the expression s contains only entry point
 */
bool entry_point(std::string s) {
  // all variables are entry points
  std::vector<std::string> all = utils::split(s, "><=+- ");
  for (std::string var : all) {
    if (var.find("input") == std::string::npos) continue;
    if (var.find("argc") == std::string::npos && var.find("argv") == std::string::npos) return false;
  }
  return true;
}

/**
 * Analyze if the failure condition is going to be satisfied only depends on the program entry point.
 */
void Analyzer::ResolveQuery(std::string failure_condition) {

  utils::trim(failure_condition);
  if (failure_condition.empty()) {
    std::cout << utils::YELLOW << "WW: failure condition is empty" << utils::RESET << "\n";
    return;
  }

  // 1. get the variables used in the failure condition
  std::map<std::string, std::vector<std::string> > mapping;
  std::set<std::string> candidate_output_var;
  std::vector<std::string> components = utils::split(failure_condition);
  for (std::string comp : components) {
    if (comp.find("output") == 0) {
      candidate_output_var.insert(comp);
    }
  }
  // 2. get the transfer functions and constant functions related to those variables
  // m_transfer_output
  std::vector<std::string> transfer_output = utils::split(m_transfer_output, '\n');
  for (std::string trans : transfer_output) {
    if (trans.find('=') != std::string::npos) {
      std::string lhs = trans.substr(0, trans.find('='));
      std::string rhs = trans.substr(trans.find('=')+1);
      utils::trim(lhs);
      utils::trim(rhs);
      if (candidate_output_var.count(lhs)) {
        if (entry_point(rhs)) {
          mapping[lhs].push_back(rhs);
        }
      }
    }
  }

  // 3. put those into Z3?? NO!
  // Now we have, for each output variables, a list of mapping
  // we try ALL combination of the mappings, until one of them contains only entry point.
  // Then, we get that condition out, as the @RETURN
  // Also, we sent this (or ALL?) into Z3 to see if it is satisfiable.

  // Try all the combinations
  if (candidate_output_var.size() == mapping.size()) {
    // std::vector<std::vector<std::string> > vv;
    // std::vector<std::vector<std::string> > combinations = get_combinations(vv);

    // TODO NOW putting into z3 to validate it
    std::cout << utils::GREEN << "== Query Resolved!" << utils::RESET << "\n";
    std::cout << "Using:" << "\n";
    for (auto m : mapping) {
      std::cout << "\t" << m.first << " = " << *m.second.begin() << "\n";
    }
    std::cout << "On top of failure condition:" << "\n";
    std::cout << "\t" << failure_condition << "\n";
    exit(0);
  } else {
    std::cout << "== Resolve failed." << "\n";
    std::cout << "The output variables in FC cannot be all mapped." << "\n";
    std::cout << "Output variables in FC: " << candidate_output_var.size() << "\n";
    std::cout << "Out of which, " << mapping.size() << " can be mapped." << "\n";
    std::cout << "The failure condition:" << "\n";
    std::cout << "\t" << failure_condition << "\n";
    std::cout << "The transfer functions:" << "\n";
    for (auto m : mapping) {
      std::cout << "\t" << m.first << " = " << *m.second.begin() << "\n";
    }
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
