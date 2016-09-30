#include "code_test.h"
#include "workflow/analyzer.h"

#include "utils/log.h"
#include "utils/utils.h"
#include "helium_options.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


#include <iostream>
#include <boost/timer/timer.hpp>
using boost::timer::cpu_timer;
using boost::timer::cpu_times;
CodeTester::CodeTester(std::string exe_folder, std::string exe, std::map<std::string, Type*> inputs)
  : m_exe_folder(exe_folder), m_exe(exe), m_inputs(inputs) {
}


void CodeTester::genTestSuite() {
  helium_print_trace("CodeTester::genTestSuite");
  int test_number = HeliumOptions::Instance()->GetInt("test-number");
  m_test_suites.clear();
  freeTestSuite();
  cpu_timer timer;
  timer.start();
  m_test_suites.resize(test_number);
  for (auto input : m_inputs) {
    std::string var = input.first;
    Type *type = input.second;
    if (!type) continue;
    
    for (int i=0;i<test_number;i++) {
      InputSpec *spec = type->GenerateInput();
      m_specs.insert(spec);
      m_test_suites[i].Add(var, spec);
    }
  }

  if (HeliumOptions::Instance()->GetBool("print-test-meta")) {
    std::cout << "Test Input Number: " << m_test_suites.size() << "\n";
    cpu_times time = timer.elapsed();
    std::cout << "Test Generation Time: " << time.wall << "\n";
  }
  
  helium_print_trace("End of CodeTester::genTestSuite");
}

void CodeTester::Test() {
  helium_print_trace("CodeTester::Test");
  genTestSuite();
  utils::create_folder((m_exe_folder / "input").string());
  utils::create_folder((m_exe_folder / "tests").string());

  cpu_timer timer;
  timer.start();

  // TODO in analyzer, calculate failure due to context
  int pass_ct=0;
  int fail_ct=0;
  for (int i=0;i<(int)m_test_suites.size();i++) {
    std::string cmd = (m_exe_folder / m_exe).string();
    int status;
    TestSuite suite = m_test_suites[i];

    // utils::write_file(m_exe_folder + "/input/" + std::to_string(i) + ".txt" + ".spec", spec);
      
    // std::string output = utils::exec_in(cmd.c_str(), test_suite[i].c_str(), &status, 10);
    // I'm also going to write the input file in the executable directory
    std::string input = suite.GetInput();
    // Run the program
    std::string output = utils::exec_in(cmd.c_str(), input.c_str(), &status, 0.3);
    utils::write_file((m_exe_folder / "input" / (std::to_string(i) + ".txt")).string(), input);

    if (status == 0) {
      if (HeliumOptions::Instance()->GetBool("print-test-info")) {
        utils::print("test success\n", utils::CK_Green);
      }
      if (HeliumOptions::Instance()->GetBool("print-test-info-dot")) {
        utils::print(".", utils::CK_Green);
      }
      pass_ct++;
      // ret->AddOutput(output, true);
      output += "HELIUM_TEST_SUCCESS = true\n";
    } else {
      fail_ct++;
      if (HeliumOptions::Instance()->GetBool("print-test-info")) {
        utils::print("test failure\n", utils::CK_Red);
      }
      if (HeliumOptions::Instance()->GetBool("print-test-info-dot")) {
        utils::print(".", utils::CK_Red);
      }
      // ret->AddOutput(output, false);
      output += "HELIUM_TEST_SUCCESS = false\n";
    }

    // For input, and output, log them out only
    fs::path in_file = m_exe_folder / "tests" / ("in-" + std::to_string(i) + ".txt");
    fs::path out_file = m_exe_folder / "tests" / ("out-" + std::to_string(i) + ".txt");
    utils::write_file(in_file.string(), suite.GetSpec());
    utils::write_file(out_file.string(), output);

  }
  if (HeliumOptions::Instance()->GetBool("print-test-info-dot")) {
    std::cout << "\n";
  }

  if (HeliumOptions::Instance()->GetBool("print-test-meta")) {
    cpu_times t = timer.elapsed();
    std::cout << "Testing Time: " << t.wall << "\n";

    // TODO coverage
    std::string cmd;
    cmd = "cd " + m_exe_folder.string() + " && gcov main.c";
    std::string out = utils::new_exec(cmd.c_str());
    fs::path gcov_file = m_exe_folder / "main.c.gcov";
    // TODO read gcov_file to get more information
    std::cout << "Coverage (gcov) Information: " << "\n";
    std::cout << out << "\n";

    std::cout << "Number of Pass Test: " << pass_ct << "\n";
    std::cout << "Number of Fail Test: " << fail_ct << "\n";

  }
  
  // return ret;
}

void CodeTester::freeTestSuite() {
  helium_print_trace("CodeTester::freeTestSuite");
  m_test_suites.clear();
  for (InputSpec *spec : m_specs) {
    delete spec;
  }
  m_specs.clear();
  helium_print_trace("End of CodeTester::freeTestSuite");
}



// void CodeTester::Analyze(TestResult *test_result) {
//   helium_print_trace("CodeTester::Analyze");
//   test_result->PrepareData();
//   // HEBI Generating CSV file
//   std::string csv = test_result->GenerateCSV();
//   // This is The whole IO file
//   // I also want to write the valid IO file
//   fs::path csv_file = m_exe_folder / "io.csv";
//   utils::write_file(csv_file.string(), csv);
//   std::cout << "Output to: " << csv_file.string()   << "\n";
//   if (HeliumOptions::Instance()->Has("print-csv")) {
//     std::string cmd = "column -s , -t " + csv_file.string();
//     std::string output = utils::exec(cmd.c_str());
//     std::cout << output  << "\n";
//   }
//   test_result->GetInvariants();
//   test_result->GetPreconditions();
//   test_result->GetTransferFunctions();
  
//   // Analyzer analyzer(csv_file, m_seg->GetConditions());
//   Analyzer analyzer(csv_file.string(), {});
//   // TODO NOW
//   // TestSummary summary = analyzer.GetSummary();
//   // if ((double)(summary.reach_poi_test_success + summary.reach_poi_test_success) / summary.total_test < 0.1) {
//   //   // hard to trigger, go to simplify approach
//   //   simplify();
//   // }
//   std::vector<std::string> invs = analyzer.GetInvariants();
//   std::vector<std::string> pres = analyzer.GetPreConditions();
//   std::vector<std::string> trans = analyzer.GetTransferFunctions();
//   if (HeliumOptions::Instance()->Has("print-analysis-result")) {
//     std::cout << "== invariants"  << "\n";
//     for (auto &s : invs) {
//       std::cout << "\t" << s  << "\n";
//     }
//     std::cout << "== pre condtions"  << "\n";
//     for (auto &s : pres) {
//       std::cout << "\t" << s  << "\n";
//     }
//     std::cout << "== transfer functions ------"  << "\n";
//     for (auto &s : trans) {
//       std::cout << "\t" << s  << "\n";
//     }

//     // std::string cmd = "compare.py -f " + csv_file;
//     // std::string inv = utils::exec(cmd.c_str());
//     // std::cout << inv  << "\n";
//   }


//   // std::cout << "---- resolveQuery -----"  << "\n";
//   // m_query_resolved = resolveQuery(invs, pres, trans);
//   // if (m_query_resolved) {
//   //   std::cout << "== Query resolved!"  << "\n";
//   //   // output some information to use in paper
//   //   std::cout << "\t sig dir: " << m_sig_dir  << "\n";
//   //   std::cout << "\t search time: " << m_search_time  << "\n";
//   // }
//   // std::cout << "------ end of query resolving -----"  << "\n";
// }


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
