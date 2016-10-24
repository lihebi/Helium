#include "tester.h"

#include "utils/log.h"
#include "utils/utils.h"
#include "helium_options.h"
#include "helper.h"
#include "type/argv.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


#include <iostream>
#include <boost/timer/timer.hpp>
using boost::timer::cpu_timer;
using boost::timer::cpu_times;
Tester::Tester(std::string exe_folder, std::string exe, std::vector<Variable*> inputs)
  : m_exe_folder(exe_folder), m_exe(exe), m_inputs(inputs) {
}


std::vector<std::pair<InputSpec*, InputSpec*> > gen_pair(std::vector<InputSpec*> v1, std::vector<InputSpec*> v2) {
  std::vector<std::pair<InputSpec*, InputSpec*> > ret;
  for (int i=0;i<(int)v1.size();i++) {
    for (int j=0;j<(int)v2.size();j++) {
      ret.push_back({v1[i], v2[j]});
    }
  }
  return ret;
}

InputSpec *random_select(std::vector<InputSpec*> v) {
  int r = utils::rand_int(0, v.size());
  // should not have the following two cases
  if (r == (int)v.size()) r--;
  assert(r < (int)v.size());
  return v[r];
}

static void setopt(std::string file) {
  std::string code = utils::read_file(file);
  if (code.find("getopt") != std::string::npos) {
    std::string opt = code.substr(code.find("getopt"));
    std::vector<std::string> lines = utils::split(opt, '\n');
    assert(lines.size() > 0);
    opt = lines[0];
    assert(opt.find("\"") != std::string::npos);
    opt = opt.substr(opt.find("\"")+1);
    assert(opt.find("\"") != std::string::npos);
    opt = opt.substr(0, opt.find("\""));
    assert(opt.find("\"") == std::string::npos);
    // print out the opt
    std::cout << utils::CYAN << opt << utils::RESET << "\n";
    // set the opt
    ArgV::Instance()->SetOpt(opt);
  }
}

static void clearopt() {
  ArgV::Instance()->ClearOpt();
}

void Tester::genRandom() {
  int test_number = HeliumOptions::Instance()->GetInt("random-test-number");
  m_test_suites.clear();
  for (int i=0;i<test_number;i++) {
    TestSuite suite;
    ArgV::Instance()->clear();
    for (Variable *var : m_inputs) {
      std::string name = var->GetName();
      if (name == "argc") {
        InputSpec *spec = ArgV::Instance()->GetArgCInput();
        suite.Add(name, spec);
      } else if (name == "argv") {
        InputSpec *spec = ArgV::Instance()->GetArgVInput();
        suite.Add(name, spec);
      } else {
        Type *type = var->GetType();
        if (!type) {
          std::cerr << "EE: when generating test suite, the type is NULL! Fatal error!" << "\n";
          exit(1);
        }
        InputSpec *spec = type->GenerateRandomInput();
        suite.Add(name, spec);
      }
    }
    m_test_suites.push_back(suite);
  }
}

void Tester::genPairwise() {
  // for each input, generate a lot of tests by their GenPair method
  // Haha
  int corner_number = HeliumOptions::Instance()->GetInt("pairwise-corner-number");
  int random_number = HeliumOptions::Instance()->GetInt("pairwise-random-number");
  std::vector<std::vector<InputSpec*> > pairpool;
  std::vector<std::string> varnames;
  for (Variable *var : m_inputs) {
    std::string name = var->GetName();
    varnames.push_back(name);
    Type *type = var->GetType();
    if (!type) {
      std::cerr << "EE: when generating test suite, the type is NULL! Fatal error!" << "\n";
      exit(1);
    }
    std::vector<InputSpec*> corner_inputs = type->GenerateCornerInputs(corner_number);
    std::vector<InputSpec*> random_inputs = type->GenerateRandomInputs(random_number);
    std::vector<InputSpec*> inputs;
    inputs.insert(inputs.end(), corner_inputs.begin(), corner_inputs.end());
    inputs.insert(inputs.end(), random_inputs.begin(), random_inputs.end());
    pairpool.push_back(inputs);
  }
  // then, for every combination of any two variable, get a combination of them, and random choose for the rest of inputs
  std::vector<TestSuite> test_suite_pool;
  if (m_inputs.size() == 1) {
    std::vector<InputSpec*> v = pairpool[0];
    for (int i=0;i<(int)v.size();i++) {
      TestSuite suite;
      suite.Add(varnames[0], v[i]);
      m_test_suites.push_back(suite);
    }
  } else {
    for (int i=0;i<(int)m_inputs.size();i++) {
      for (int j=i+1;j<(int)m_inputs.size();j++) {
        // pairwise i and j
        std::vector<std::pair<InputSpec*, InputSpec*> > pairs = gen_pair(pairpool[i], pairpool[j]);
        for (int m=0;m<(int)pairs.size();m++) {
          std::pair<InputSpec*, InputSpec*> p = pairs[m];
          TestSuite suite;
          for (int n=0;n<(int)m_inputs.size();n++) {
            if (n == i) {
              suite.Add(varnames[n], p.first);
            } else if (n == j) {
              suite.Add(varnames[n], p.second);
            } else {
              suite.Add(varnames[n], random_select(pairpool[n]));
            }
          }
          test_suite_pool.push_back(suite);
        }
      }
    }
  }
  // select from the pool
  int pairwise_test_number = HeliumOptions::Instance()->GetInt("pairwise-test-number");
  if (pairwise_test_number < (int)test_suite_pool.size()) {
    std::set<int> rv = utils::rand_ints(0, test_suite_pool.size(), pairwise_test_number);
    for (int r : rv) {
      m_test_suites.push_back(test_suite_pool[r]);
    }
  } else {
    m_test_suites.insert(m_test_suites.end(), test_suite_pool.begin(), test_suite_pool.end());
  }
}

void Tester::genTestSuite() {
  helium_print_trace("Tester::genTestSuite");
  if (m_inputs.size() == 0) {
    return;
  }
  freeTestSuite();
  m_test_suites.clear();
  std::string method = HeliumOptions::Instance()->GetString("test-generation-method");
  
  // scan the code and set opt
  setopt((m_exe_folder / "main.c").string());
  // (HEBI: generate input)
  if (method == "random") {
    genRandom();
  } else if (method == "pairwise") {
    genPairwise();
  } else {
    std::cerr << "EE: unsupported test generation method: " << method << "\n";
  }
  clearopt();
}

void Tester::Test() {
  helium_print_trace("Tester::Test");
  cpu_timer timer;
  timer.start();
  genTestSuite();

  if (HeliumOptions::Instance()->GetBool("pause-no-testcase")) {
    if (m_test_suites.empty()) {
      std::cout << "Paused because pause-no-testcase set to true. Press enter to continue..." << "\n";
      getchar();
    }
  }
  cpu_times test_gen_time = timer.elapsed();

  if (HeliumOptions::Instance()->GetBool("print-test-meta")) {
    std::cout << utils::PURPLE << "Test Meta:" << utils::RESET << "\n";
    std::cout << "\t" << "Number of input variables: " << m_inputs.size() << "\n";
    std::cout << "\t" << "Number of tests: " << m_test_suites.size() << "\n";
    std::cout << "\t" << "Test Generation Time: " << boost::timer::format(test_gen_time, 3, "%w seconds") << "\n";
  }


  utils::create_folder((m_exe_folder / "input").string());
  utils::create_folder((m_exe_folder / "tests").string());

  // TODO in analyzer, calculate failure due to context
  int pass_ct=0;
  int fail_ct=0;

  
  fs::path test_result_file = m_exe_folder / "result.txt";
  fs::path test_input_file = m_exe_folder / "input.txt";
  
  for (int i=0;i<(int)m_test_suites.size();i++) {
    std::string cmd = (m_exe_folder / m_exe).string();
    int status;
    TestSuite suite = m_test_suites[i];

    // utils::write_file(m_exe_folder + "/input/" + std::to_string(i) + ".txt" + ".spec", spec);
      
    // std::string output = utils::exec_in(cmd.c_str(), test_suite[i].c_str(), &status, 10);
    // I'm also going to write the input file in the executable directory
    std::string input = suite.GetInput();


    // DEPRECATED no longer use the spec, instrument in the code instead
    // std::cout << suite.GetSpec() << "\n";


    // (HEBI: Run the program)
    if (HeliumOptions::Instance()->Has("verbose")) {
      std::cout << "Running the program ..." << "\n";
    }

    int timeout_ms = HeliumOptions::Instance()->GetInt("test-timeout");
    float timeout_s = (float)timeout_ms / 1000;
    std::string output = utils::exec_in(cmd.c_str(), input.c_str(), &status, timeout_s);
    if (HeliumOptions::Instance()->Has("verbose")) {
      std::cout << "End of running" << "\n";
    }


    utils::write_file((m_exe_folder / "input" / (std::to_string(i) + ".txt")).string(), input);

    if (status == 0) {
      if (HeliumOptions::Instance()->GetBool("print-test-info")) {
        std::cout << utils::GREEN << "test success\n" << utils::RESET << "\n";
      }
      if (HeliumOptions::Instance()->GetBool("print-test-info-dot")) {
        std::cout << utils::GREEN  << "." << utils::RESET << std::flush;
      }
      pass_ct++;
      // ret->AddOutput(output, true);
      output += "HELIUM_TEST_SUCCESS\n";
    } else {
      fail_ct++;
      if (HeliumOptions::Instance()->GetBool("print-test-info")) {
        std::cout << utils::RED << "test failure\n" << utils::RESET << "\n";

      }
      if (HeliumOptions::Instance()->GetBool("print-test-info-dot")) {
        std::cout << utils::RED << "." << utils::RESET << std::flush;
      }
      // ret->AddOutput(output, false);
      output += "HELIUM_TEST_FAILURE\n";
    }


    // DEBUG
    // if (HeliumOptions::Instance()->Has("verbose")) {
    //   std::cout << output << "\n";
    // }

    // For input, and output, log them out only
    // fs::path in_file = m_exe_folder / "tests" / ("in-" + std::to_string(i) + ".txt");
    // fs::path out_file = m_exe_folder / "tests" / ("out-" + std::to_string(i) + ".txt");
    // utils::write_file(in_file.string(), suite.GetSpec());
    // utils::write_file(out_file.string(), output);

    utils::append_file(test_result_file.string(), output);
    utils::append_file(test_input_file.string(), "-----\n" + suite.GetSpec());
 
  }
  if (HeliumOptions::Instance()->GetBool("print-test-info-dot")) {
    std::cout << "\n";
  }
  cpu_times test_total = timer.elapsed();

  if (HeliumOptions::Instance()->GetBool("print-test-meta")) {
    std::cout << "\t" << "Total Testing Time: " << boost::timer::format(test_total, 3, "%w seconds") << "\n";
    // only calculate coverage if we run some tests
    if (!m_test_suites.empty()) {
      Gcov gcov(m_exe_folder, "main.c");
      std::cout << "\t" << "Stmt Coverage: " << gcov.GetStmtCoverage() << "\n";
      std::cout << "\t" << "Branch Coverage: " << gcov.GetBranchCoverage() << "\n";
    }

    std::cout << "\t" << "Number of Pass Test: " << pass_ct << "\n";
    std::cout << "\t" << "Number of Fail Test: " << fail_ct << "\n";

  }
  
  // return ret;
}

void Tester::freeTestSuite() {
  helium_print_trace("Tester::freeTestSuite");
  m_test_suites.clear();
  for (InputSpec *spec : m_specs) {
    delete spec;
  }
  m_specs.clear();
  helium_print_trace("End of Tester::freeTestSuite");
}




std::string TestSuite::GetInput() {
  std::string ret;
  for (auto &m : m_data) {
    std::string var = m.first;
    InputSpec *spec = m.second;
    if (spec) {
      ret += spec->GetRaw() + "\n";
    }
  }
  return ret;
}


std::string TestSuite::GetSpec() {
  std::string ret;
  for (auto &m : m_data) {
    std::string var = m.first;
    InputSpec *spec = m.second;
    if (spec) {
      ret += var + ": " + spec->GetSpec() + "\n";
    }
  }
  return ret;
}













// void Tester::Analyze(TestResult *test_result) {
//   helium_print_trace("Tester::Analyze");
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


// bool Tester::resolveQuery(std::vector<std::string> str_invs, std::vector<std::string> str_pres, std::vector<std::string> str_trans) {
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
