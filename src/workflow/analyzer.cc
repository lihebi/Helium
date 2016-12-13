#include "analyzer.h"
#include "utils/utils.h"
#include "helium_options.h"
#include <iostream>
#include <gtest/gtest.h>

static const std::string TRANSFER_SCRIPT = "helium-analyze-transfer.R";
static const std::string META_SCRIPT = "helium-analyze-meta.R";


static const std::string TRANSFER_SCRIPT_TIMEOUT = "helium-analyze-transfer-timeout.sh";
static const std::string META_SCRIPT_TIMEOUT = "helium-analyze-meta-timeout.sh";

void Analyzer::GetCSV() {
  // call the script helium-output-to-csv.py
  // std::string cmd = "helium-output-to-csv.py " + m_dir + "/result.txt > " + m_dir + "/result.csv";
  // utils::new_exec(cmd.c_str());
  // std::cout << "generated CSV file" << "\n";

  // DEBUG print out the csv file
  // cmd = "cat " + m_dir + "/result.csv";
  // std::string output = utils::new_exec(cmd.c_str());
  // if (HeliumOptions::Instance()->Has("verbose")) {
  //   std::cout << output << "\n";
  // }
  // std::cout << output << "\n";
}

void Analyzer::AnalyzeCSV() {
  std::string result_csv = m_dir + "/result.csv";
  std::string cmd;
  // transfer function
  // using timeout version. The default is 3s
  cmd = TRANSFER_SCRIPT_TIMEOUT + " " + result_csv;
  // std::cout << "Analyzing using linear regression .." << "\n";
  // std::cout << cmd << "\n";
  m_transfer_output = utils::new_exec(cmd.c_str());

  cmd = META_SCRIPT_TIMEOUT + " " + result_csv;
  m_meta_output = utils::new_exec(cmd.c_str());

  
  // std::cout << "Transfer functions:" << "\n";
  if (HeliumOptions::Instance()->GetBool("print-analyze-result-transfer")) {
    std::cout << utils::PURPLE << "Result Transfer Function: " << utils::RESET << "\n";
    std::cout << utils::indent_string(m_transfer_output) << "\n";
  }

  if (HeliumOptions::Instance()->GetBool("print-analyze-result-meta")) {
    std::cout << utils::PURPLE << "Result Meta: " << utils::RESET << "\n";
    std::cout << utils::indent_string(m_meta_output) << "\n";
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
    if (var.find("argc") == std::string::npos
        && var.find("argv") == std::string::npos
        // hard code this as entry point
        // TODO needs an instrumentation to tell whether a point is entry point
        && var.find("entry") == std::string::npos) return false;
  }
  return true;
}

static bool is_constant(std::string s) {
  std::vector<std::string> all = utils::split(s, "><=+- ");
  for (std::string var : all) {
    if (var.find("input") != std::string::npos
        || var.find("output") != std::string::npos) {
      return false;
    }
  }
  return true;
}





std::string get_declare_fun(std::string var) {
  if (var.find("int") != std::string::npos) {
    return "(declare-fun " + var + "() Int)";
  } else if (var.find("bool") != std::string::npos) {
    return "(declare-fun " + var + "() Bool)";
  } else {
    return "(declare-fun " + var + "() Real)";
  }
}

std::string get_rhs(std::string s) {
  if (s.find_first_of("+-*") != std::string::npos) {
    std::string ret;
    int idx = s.find_first_of("+-*");
    std::string lhs = s.substr(0,idx);
    std::string rhs = s.substr(idx+1);
    utils::trim(lhs);
    utils::trim(rhs);
    char op = s[idx];
    ret += "(" + std::string(1, op) + " " + lhs + " ";
    ret += get_rhs(rhs) + ")";
    return ret;
  } else {
    return s;
  }
}

std::string get_assert(std::string constraint, bool negate=false) {
  std::string ret;
  ret += "(assert ";
  if (negate) {
    ret += "(not ";
  }
  // only support binary operation
  std::vector<std::string> hs = utils::split(constraint, "<=>");
  if (hs.size() != 2) {
    std::cerr << "not binary: " << constraint << "\n";
    std::cout << "actuall size: " << hs.size() << "\n";
    return "(assert (= 0 1))";
  }

  // if (constraint.find('=') != std::string::npos) {
  //   ret += "(= ";
  // } else if (constraint.find(">") != std::string::npos) {
  //   ret += "(> ";
  // } else {
  //   ret += "(< ";
  // }
  std::string lhs = hs[0];
  std::string rhs = hs[1];
  utils::trim(lhs);
  utils::trim(rhs);
  rhs = get_rhs(rhs);
  if (constraint.find(">=") != std::string::npos) {
    ret += "(>= " + lhs + " " + rhs + ")";
  } else if (constraint.find("<=") != std::string::npos) {
    ret += "(<= " + lhs + " " + rhs + ")";
  } else if (constraint.find("==") != std::string::npos) {
    ret += "(= " + lhs + " " + rhs + ")";
  } else if (constraint.find("!=") != std::string::npos) {
    ret += "(not (= " + lhs + " " + rhs + "))";
  } else if (constraint.find("=") != std::string::npos) {
    ret += "(= " + lhs + " " + rhs + ")";
  } else if (constraint.find(">") != std::string::npos) {
    ret += "(> " + lhs + " " + rhs + ")";
  } else if (constraint.find("<") != std::string::npos) {
    ret += "(< " + lhs + " " + rhs + ")";
  } else {
    // FIXME this might cause false positive!
    ret =+ "(assert (= 0 1))";
  }
  if (negate) {
    ret += ")";
  }
  ret += ")";
  return ret;
}

TEST(AnalyzerCase, AssertTest) {
  std::string s1 = "a=b+1";
  std::string s2 = "a<=b";
  EXPECT_EQ(get_assert("a=b+1"), "(assert (= a (+ b 1)))");
  EXPECT_EQ(get_assert("a<=b"), "(assert (<= a b))");
}

std::vector<std::string> escape(std::vector<std::string> v) {
  std::vector<std::string> ret;
  for (std::string s : v) {
    utils::replace(s, "[", ".");
    utils::replace(s, "]", ".");
    ret.push_back(s);
  }
  return ret;
}

bool check_sat(std::vector<std::string> v, std::vector<std::string> vneg={}) {
  // escape
  v = escape(v);
  vneg = escape(vneg);
  std::vector<std::string> vall;
  vall.insert(vall.begin(), v.begin(), v.end());
  vall.insert(vall.begin(), vneg.begin(), vneg.end());
  // get variables
  std::set<std::string> vars;
  for (std::string s : vall) {
    std::vector<std::string> all = utils::split(s, "><=+- ");
    for (std::string var : all) {
      if (var.find("output") == 0 || var.find("input") == 0) {
        vars.insert(var);
      }
    }
  }
  
  // create variable statements
  std::string smt;
  smt += "(declare-const nil Int)\n";
  // FIXME multiple oneofargv would need multiple different SMT variable name
  // TODO more general way to automatically insert based on "const-like" variable name
  smt += "(declare-const oneofargv Int)\n";
  for (std::string var : vars) {
    smt += get_declare_fun(var) + "\n";
  }
  
  // create checking statements
  for (std::string s : v) {
    smt += get_assert(s) + "\n";
  }
  for (std::string s : vneg) {
    smt += get_assert(s, true) + "\n";
  }
  smt += "(check-sat)";
  
  // write to a smt file

  if (HeliumOptions::Instance()->GetBool("print-sat-stmt")) {
    std::cout << smt << "\n";
  }
  std::string dir = utils::create_tmp_dir();
  std::string smt_file = dir + "/helium.smt";
  utils::write_file(smt_file, smt);
  // std::cout << "output smt to " << smt_file<< "\n";
  // call z3 to execute
  // FIXME check whether z3 is available
  std::string cmd = "z3 -v:1 -smt2 " + smt_file;
  std::string output = utils::new_exec(cmd.c_str());
  utils::trim(output);
  if (output == "sat") {
    return true;
  } else {
    if (HeliumOptions::Instance()->GetBool("print-sat-output")) {
      std::cout << "SAT output:" << "\n";
      std::cout << output << "\n";
    }
    return false;
  }
}

TEST(SATCase, SATTest) {
  std::vector<std::string> v;
  v.push_back("output_int_x > 8");
  v.push_back("output_int_y < 7");
  EXPECT_TRUE(check_sat(v));
  v.push_back("output_int_y > output_int_x");
  EXPECT_FALSE(check_sat(v));
  v.pop_back();
  std::vector<std::string> vneg;
  vneg.push_back("output_int_x > 8");
  EXPECT_FALSE(check_sat(v, vneg));
}



/**
 * Analyze if the failure condition is going to be satisfied only depends on the program entry point.
 */
bool Analyzer::ResolveQuery(std::string failure_condition) {

  utils::trim(failure_condition);
  if (failure_condition.empty()) {
    std::cout << utils::YELLOW << "WW: failure condition is empty" << utils::RESET << "\n";
    return false;
  }

  // 1. get the variables used in the failure condition
  std::set<std::string> candidate_output_var;
  std::vector<std::string> components = utils::split(failure_condition);
  for (std::string comp : components) {
    if (comp.find("output") == 0) {
      candidate_output_var.insert(comp);
    }
  }
  // 2. get the transfer functions and constant functions related to those variables
  // store the mapping to entry point
  std::map<std::string, std::vector<std::string> > mapping;
  // store the mapping not necessary to entry point. Used for merging query
  // Also use approximation: only store the first one
  // std::map<std::string, std::string> mapping_no_need_entry;
  m_used_transfer.clear();
  // m_transfer_output
  std::vector<std::string> transfer_output = utils::split(m_transfer_output, '\n');
  for (std::string trans : transfer_output) {
    if (trans.find('=') != std::string::npos) {
      std::string lhs = trans.substr(0, trans.find('='));
      std::string rhs = trans.substr(trans.find('=')+1);
      utils::trim(lhs);
      utils::trim(rhs);
      if (candidate_output_var.count(lhs)) {
        // std::cout << "for variable " << lhs << "\n";
        // std::cout << "we have " << rhs << "\n";
        // std::cout << candidate_output_var.size() << "\n";
        // std::cout << is_constant(rhs) << "\n";
        if (candidate_output_var.size() == 1 && is_constant(rhs)) {
          // std::cout << "but it is removed" << "\n";
          // this is for output_addr_y=nil
          // This is true, but we need to find the condition!
          // FIXME this will be wrong if the condition is indeed incured by the path, and it satisfied the output
          // so the thing missing is: only one output variable, and the variable is determistic (do not have a transfer from input)
          continue;
        }
        if (m_used_transfer.count(lhs)==0) {
          m_used_transfer[lhs] = rhs;
        }
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

    std::cout << "Using:" << "\n";
    std::vector<std::string> v;
    v.push_back(failure_condition);
    for (auto m : mapping) {
      // only using the first function
      std::cout << "\t" << m.first << " = " << *m.second.begin() << "\n";
      v.push_back(m.first + " = " + *m.second.begin());
    }
    std::cout << "On top of failure condition:" << "\n";
    std::cout << "\t" << failure_condition << "\n";
    if (check_sat(v)) {
      std::cout << utils::GREEN << "== Query Resolved!" << utils::RESET << "\n";
      return true;
    } else {
      std::cout << utils::RED << "== SAT unsatisfiable." << utils::RESET << "\n";
    }
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
    std::cout << "All used transfer funtions:" << "\n";
    for (auto m : m_used_transfer) {
      std::cout << "\t" << m.first << " = " << m.second << "\n";
    }
  }
  return false;
}

bool check_fc(std::vector<std::string> cons, std::string fc) {
  cons.push_back(fc);
  return check_sat(cons);
}

bool check_negfc(std::vector<std::string> cons, std::string fc) {
  std::vector<std::string> neg;
  neg.push_back(fc);
  return check_sat(cons, neg);
}

/**
 * Resolve query using only constraint solver.
 * But how to get the model?
 * How to judge the entry point?
 */
bool Analyzer::ResolveQuery2(std::string failure_condition) {
  utils::trim(failure_condition);
  if (failure_condition.empty()) {
    std::cout << utils::YELLOW << "WW: failure condition is empty" << utils::RESET << "\n";
    return false;
  }

  // 1. get the variables used in the failure condition
  std::set<std::string> candidate_output_var;
  std::vector<std::string> components = utils::split(failure_condition);
  for (std::string comp : components) {
    if (comp.find("output") == 0) {
      candidate_output_var.insert(comp);
    }
  }
  // get the transfer functions related to them
  std::vector<std::string> transfer_output = utils::split(m_transfer_output, '\n');
  std::vector<std::string> transfer_functions;
  for (std::string trans : transfer_output) {
    if (trans.find('=') != std::string::npos) {
      transfer_functions.push_back(trans);
    }
  }
  // put failure condition and all transfer functions
  // put the negation of failure condition?

  // failure condition + all trans
  // 1. [semi-good] sat:
  //    If all mapped to entry point:
  //    RESOLVED: the failure can be triggered. 
  // 2. [good] unsat: RESOLVED: the failure cannot be triggered
  // assertion (negate) + all trans
  // 1. [bad] sat: useless
  // 2. [good] unsat: RESOLVED: the failure always trigger

  if (check_fc(transfer_functions, failure_condition)) {
    // need to check entry point
    std::cout << "FC satisfiable. Need to check entry point" << "\n";
  } else {
    // resolved
    std::cout << utils::GREEN
              << "RESOLVED: FC cannot be satisfied. The failure cannot be triggered."
              << utils::RESET<< "\n";
    return true;
  }
  if (check_negfc(transfer_functions, failure_condition)) {
    // useless
  } else {
    std::cout << utils::GREEN
              << "RESOLVED: FC always satisfied"
              << utils::RESET << "\n";
    return true;
  }
  return false;
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



bool Analyzer::same_trans(Analyzer *p1, Analyzer *p2) {
  if (!p1 || !p2) return false;

  std::map<std::string, std::string> t1 = p1->GetUsedTransfer();
  std::map<std::string, std::string> t2 = p2->GetUsedTransfer();
  
  if (t1.size() != t2.size()) {
    return false;
  }
  for (auto m : t1) {
    if (t2.count(m.first) == 0 || t2[m.first] != m.second) {
      return false;
    }
  }
  return true;
    
}

void Analyzer::print_used_trans(Analyzer *p) {
  if (p) {
    std::map<std::string, std::string> t = p->GetUsedTransfer();
    for (auto m : t) {
      std::cout << m.first << " " << m.second << "\n";
    }
  }
}
