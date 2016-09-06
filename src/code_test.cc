#include "code_test.h"
#include "config/config.h"
#include "config/options.h"
#include "workflow/analyzer.h"



#include <iostream>


void CodeTester::genTestSuite() {
  print_trace("CodeTester::genTestSuite");
  // FIXME this function is too long
  if (Config::Instance()->GetBool("run-test") == false) {
    return;
  }
  int test_number = Config::Instance()->GetInt("test-number");
  // std::vector<std::vector<InputSpec*> > test_suite(test_number);
  m_test_suite.clear();
  freeTestSuite();
  m_test_suite.resize(test_number);
  // decl_deco decl_input_m = m_ast_to_deco_m[m_first->GetAST()].second;

  /**
   * Testing!
   */
  // print_trace("preparing inputs ...");
  // FIXME decls or inputs?
  // InputMetrics metrics = m_inputs[first_ast];
  // metrics.insert(m_global.begin(), m_global.end());
  for (auto input : m_inputs) {
    std::string var = input.first;
    Type *type = input.second;
    if (!type) continue;
    
    std::vector<InputSpec*> suite;

    for (int i=0;i<test_number;i++) {
      InputSpec *spec = type->GenerateInput();
      m_test_suite[i].push_back(spec);
    }
    // assert((int)inputs.size() == test_number);
    // for (int i=0;i<(int)inputs.size();i++) {
    //   m_test_suite[i].push_back(inputs[i]);
    // }
  }
  print_trace("End of CodeTester::genTestSuite");
}

TestResult* CodeTester::Test() {
  print_trace("CodeTester::Test");
  // std::cout << "generate test suite" << "input size: " << m_inputs.size()  << "\n";
  genTestSuite();
  TestResult *ret = new TestResult(m_test_suite);

  // std::string test_dir = utils::create_tmp_dir();
  utils::create_folder(m_exe_folder + "/input");
  for (int i=0;i<(int)m_test_suite.size();i++) {
    // std::cout << "test " << i  << "\n";
    // std::string test_file = test_dir + "/test" + std::to_string(i) + ".txt";
    // utils::write_file(test_file, m_test_suite[i]);
    // std::string cmd = m_builder->GetExecutable() + "< " + test_file + " 2>/dev/null";
    // std::cout << cmd  << "\n";
    std::string cmd = m_exe;
    int status;
    // FIXME some command cannot be controled by time out!
    // std::string output = utils::exec(cmd.c_str(), &status, 1);
    std::string input;
    std::string spec;
    for (InputSpec *in : m_test_suite[i]) {
      if (in) {
        input += in->GetRaw() + "\n";
        // spec += in->dump() + "\n";
        // spec += in->ToString() + "\n\n";
      }
    }

    utils::write_file(m_exe_folder + "/input/" + std::to_string(i) + ".txt" + ".spec", spec);
      
    // std::string output = utils::exec_in(cmd.c_str(), test_suite[i].c_str(), &status, 10);
    // I'm also going to write the input file in the executable directory
    utils::write_file(m_exe_folder + "/input/" + std::to_string(i) + ".txt", input);
    std::string output = utils::exec_in(cmd.c_str(), input.c_str(), &status, 0.3);
    if (status == 0) {
      if (PrintOption::Instance()->Has(POK_TestInfo)) {
        utils::print("test success\n", utils::CK_Green);
      }
      if (PrintOption::Instance()->Has(POK_TestInfoDot)) {
        utils::print(".", utils::CK_Green);
      }
      ret->AddOutput(output, true);
    } else {
      if (PrintOption::Instance()->Has(POK_TestInfo)) {
        utils::print("test failure\n", utils::CK_Red);
      }
      if (PrintOption::Instance()->Has(POK_TestInfoDot)) {
        utils::print(".", utils::CK_Red);
      }
      ret->AddOutput(output, false);
    }
  }
  if (PrintOption::Instance()->Has(POK_TestInfoDot)) {
    std::cout << "\n";
  }
  return ret;
}

void CodeTester::freeTestSuite() {
  print_trace("CodeTester::freeTestSuite");
  for (std::vector<InputSpec*> &v : m_test_suite) {
    for (InputSpec *in : v) {
      if (in) {
        delete in;
      }
    }
  }
  m_test_suite.clear();
  print_trace("End of CodeTester::freeTestSuite");
}



void CodeTester::Analyze(TestResult *test_result) {
  print_trace("CodeTester::Analyze");
  test_result->PrepareData();
  // HEBI Generating CSV file
  std::string csv = test_result->GenerateCSV();
  // This is The whole IO file
  // I also want to write the valid IO file
  std::string csv_file = m_exe_folder + "/io.csv";
  utils::write_file(csv_file, csv);
  std::cout << "Output to: " << csv_file   << "\n";
  if (PrintOption::Instance()->Has(POK_CSV)) {
    std::string cmd = "column -s , -t " + csv_file;
    std::string output = utils::exec(cmd.c_str());
    std::cout << output  << "\n";
  }
  test_result->GetInvariants();
  test_result->GetPreconditions();
  test_result->GetTransferFunctions();
  
  // Analyzer analyzer(csv_file, m_seg->GetConditions());
  Analyzer analyzer(csv_file, {});
  // TODO NOW
  // TestSummary summary = analyzer.GetSummary();
  // if ((double)(summary.reach_poi_test_success + summary.reach_poi_test_success) / summary.total_test < 0.1) {
  //   // hard to trigger, go to simplify approach
  //   simplify();
  // }
  std::vector<std::string> invs = analyzer.GetInvariants();
  std::vector<std::string> pres = analyzer.GetPreConditions();
  std::vector<std::string> trans = analyzer.GetTransferFunctions();
  if (PrintOption::Instance()->Has(POK_AnalysisResult)) {
    std::cout << "== invariants"  << "\n";
    for (auto &s : invs) {
      std::cout << "\t" << s  << "\n";
    }
    std::cout << "== pre condtions"  << "\n";
    for (auto &s : pres) {
      std::cout << "\t" << s  << "\n";
    }
    std::cout << "== transfer functions ------"  << "\n";
    for (auto &s : trans) {
      std::cout << "\t" << s  << "\n";
    }

    // std::string cmd = "compare.py -f " + csv_file;
    // std::string inv = utils::exec(cmd.c_str());
    // std::cout << inv  << "\n";
  }


  // std::cout << "---- resolveQuery -----"  << "\n";
  // m_query_resolved = resolveQuery(invs, pres, trans);
  // if (m_query_resolved) {
  //   std::cout << "== Query resolved!"  << "\n";
  //   // output some information to use in paper
  //   std::cout << "\t sig dir: " << m_sig_dir  << "\n";
  //   std::cout << "\t search time: " << m_search_time  << "\n";
  // }
  // std::cout << "------ end of query resolving -----"  << "\n";
}


// bool CodeTester::resolveQuery(std::vector<std::string> str_invs, std::vector<std::string> str_pres, std::vector<std::string> str_trans) {
//   // Construct binary forumlas
//   std::vector<BinaryFormula*> invs;
//   std::vector<BinaryFormula*> pres;
//   std::vector<BinaryFormula*> trans;
//   // create BinaryFormula here. CAUTION need to free them
//   for (std::string &s : str_invs) {
//     invs.push_back(new BinaryFormula(s));
//   }
//   for (std::string &s : str_pres) {
//     pres.push_back(new BinaryFormula(s));
//   }
//   for (std::string &s : str_trans) {
//     trans.push_back(new BinaryFormula(s));
//   }
//   if (invs.empty() || trans.empty() || pres.empty()) return false;
//   // how to identify the key invariant?
//   // the constant invariants is used for derive
//   // the relationship invariants are used as key
//   // any relationship invariants is enough, for now
//   BinaryFormula *key_inv = get_key_inv(invs);
//   std::cout << "| selected the key invariants: " << key_inv->dump()  << "\n";
  
//   // STEP Then, find the variables used in the key invariant
//   // FIXME No, I need the whole header, e.g. strlen(fileptr). The sizeof(fileptr) is not useful at all.
//   // find the related transfer functions
//   // std::set<std::string> vars = key_inv->GetVars();
//   std::vector<BinaryFormula*> related_trans; // = get_related_trans(trans, vars);
//   for (BinaryFormula *bf : trans) {
//     // std::string output_var = bf->GetLHSVar();
//     // if (vars.count(output_var) == 1) {
//     //   related_trans.push_back(bf);
//     // }
//     std::string item = bf->GetLHS();
//     if (item == key_inv->GetLHS() || item == key_inv->GetRHS()) {
//       related_trans.push_back(bf);
//     }
//   }

//   // find the preconditions that define the input variables used in the transfer function
//   std::set<std::string> related_input_items;
//   for (BinaryFormula* bf : related_trans) {
//     // FIXME y = x + z; I only want x
//     related_input_items.insert(bf->GetRHS());
//   }
//   std::vector<BinaryFormula*> related_pres;
//   for (BinaryFormula *bf : pres) {
//     if (related_input_items.count(bf->GetLHS()) == 1 || related_input_items.count(bf->GetRHS()) == 1) {
//       related_pres.push_back(bf);
//     }
//   }
//   // use the preconditions and transfer funcitons to derive the invariants
//   BinaryFormula* used_pre = derive_key_inv(related_pres, related_trans, key_inv);
//   if (used_pre) {
//     std::cout << "| Found the precondition that can derive to the invariant: " << used_pre->dump()  << "\n";
//     // examine the variables in those preconditons, to see if they are entry points
//     std::set<std::string> used_vars = used_pre->GetVars();
//     // FIXME ugly
//     free_binary_formula(pres);
//     free_binary_formula(trans);
//     free_binary_formula(invs);
//     // for (BinaryFormula *bf : used_pres) {
//     //   used_vars.insert(bf->GetLHSVar());
//     //   used_vars.insert(bf->GetRHSVar());
//     // }
//     std::cout << "| The variables used: "  << "\n";
//     for (std::string var : used_vars) {
//       std::cout << "| " << var  << "\n";
//       // this is argv:f!
//       if (var.find(':') != std::string::npos) {
//         var = var.substr(0, var.find(':'));
//       }
//       if (var != "argv" && var != "argc" && var != "optarg") {
//         return false;
//       }
//       if (var == "argv" || var == "argc") {
//         // the argv should come from main
//         if (m_first->GetAST()->GetFunctionName() != "main") {
//           return false;
//         }
//       }
//     }
//     utils::print("Context Searching Success!\n", utils::CK_Green);
//     return true;
//   } else {
//     free_binary_formula(pres);
//     free_binary_formula(trans);
//     free_binary_formula(invs);
//     return false;
//   }
// }
