#include "helium.h"
#include <string>
#include <iostream>

#include "reader.h"
#include "helium_utils.h"

#include "helium_options.h"
#include "parser/xml_doc_reader.h"
#include "parser/xmlnode.h"
#include "parser/ast_node.h"
#include "parser/cfg.h"

#include "utils/utils.h"
#include "utils/dump.h"

#include "resolver/snippet.h"
#include "resolver/resolver.h"
#include "resolver/snippet_db.h"
#include "resource.h"
#include "utils/log.h"
#include "builder.h"
#include "analyzer.h"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>


#include "code_test.h"
#include "code_gen.h"
#include "code_analyze.h"

namespace fs = boost::filesystem;


using namespace utils;



Helium::Helium(PointOfInterest *poi) {
  XMLDoc *doc = XMLDocReader::Instance()->ReadFile(poi->GetFolder());
  int linum = get_true_linum(poi->GetFolder(), poi->GetLinum());
  std::cout << "true line number: " <<  linum  << "\n";
  if (poi->GetType() == "stmt") {
    XMLNode node = find_node_on_line(doc->document_element(),
                                          {NK_Stmt, NK_ExprStmt, NK_DeclStmt, NK_Return, NK_Break, NK_Continue, NK_Return},
                                          linum);
    assert(node);
    XMLNode func = get_function_node(node);
    std::string func_name = function_get_name(func);
    int func_linum = get_node_line(func);
    AST *ast = Resource::Instance()->GetAST(func_name);
    if (!ast) {
      helium_log("[WW] No AST constructed!");
      return;
    }
    ASTNode *root = ast->GetRoot();
    if (!root) {
      helium_log("[WW] AST root is NULL.");
      return;
    }
    int ast_linum = root->GetBeginLinum();
    int target_linum = linum - func_linum + ast_linum;
    ASTNode *target = ast->GetNodeByLinum(target_linum);
    if (!target) {
      helium_log("[WW] cannot find target AST node");
      return;
    }
    process(target);
  }
}

// Helium::Helium(FailurePoint *fp) {
//   helium_print_trace("hebi");
//   XMLDoc *doc = XMLDocReader::Instance()->ReadFile(fp->GetPath());
//   int linum = get_true_linum(fp->GetPath(), fp->GetLinum());
//   std::cout << "true line number: " <<  linum  << "\n";
//   if (fp->GetType() == "stmt") {
//     XMLNode node = find_node_on_line(doc->document_element(),
//                                           {NK_Stmt, NK_ExprStmt, NK_DeclStmt, NK_Return, NK_Break, NK_Continue, NK_Return},
//                                           linum);
//     assert(node);
//     XMLNode func = get_function_node(node);
//     std::string func_name = function_get_name(func);
//     int func_linum = get_node_line(func);
//     AST *ast = Resource::Instance()->GetAST(func_name);
//     if (!ast) {
//       helium_log("[WW] No AST constructed!");
//       return;
//     }
//     ASTNode *root = ast->GetRoot();
//     if (!root) {
//       helium_log("[WW] AST root is NULL.");
//       return;
//     }
//     int ast_linum = root->GetBeginLinum();
//     int target_linum = linum - func_linum + ast_linum;
//     ASTNode *target = ast->GetNodeByLinum(target_linum);
//     if (!target) {
//       helium_log("[WW] cannot find target AST node");
//       return;
//     }
//     process(target);
//   }
// }


/**
 * 
 * Build a single segment to get the error condition.
 * - Build the query
 * - GenerateInput
 * - Test
 * - Oracle infer failure condition
 * This will fill failure condition
 */
void Helium::init(ASTNode *node) {
  Query *query = new Query(node);

  query->ResolveInput();
  query->GenCode();
  Builder builder;
  builder.SetMain(query->GetMain());
  builder.SetSupport(query->GetSupport());
  builder.SetMakefile(query->GetMakefile());
  builder.Write();
  builder.Compile();
  std::cout << "Code Written to: " << builder.GetDir()  << "\n";
  if (builder.Success()) {
    utils::print("compile success\n", utils::CK_Green);
    std::string executable = builder.GetExecutable();
  } else {
    utils::print("compile error\n", utils::CK_Red);
    std::cerr << "EE: The selected Failure Point is not able to compile"  << "\n";
    exit(1);
  }
  CodeTester tester(builder.GetDir(), builder.GetExecutable(), query->GetInputs());
  tester.Test();
  CodeAnalyzer new_analyzer(builder.GetDir());
  new_analyzer.Compute();

  Analyzer analyzer(builder.GetDir() + "/io.csv", {});
  std::vector<std::string> invs = analyzer.GetInvariants();
  // invs
  std::cout << "Failure invariants:"  << "\n";
  // merge
  for (std::string inv : invs) {
    // TODO many?
    std::cout << inv  << "\n";
  }
  if (invs.empty()) {
    std::cerr << "EE: Failed to infer failure condition."  << "\n";
    exit(1);
  }

  m_failure_condition = merge_failure_condition(invs);
  std::cout << "******************************"  << "\n";
  std::cout << "selected failure condition:"  << "\n";
  std::cout << "\t" << m_failure_condition  << "\n";
  std::cout << "******************************"  << "\n";
  if (m_failure_condition.empty()) {
    std::cerr << "EE: Failure condition empty."  << "\n";
    exit(1);
  }
}

std::string Helium::merge_failure_condition(std::vector<std::string> str_invs) {
  if (str_invs.size() == 1) return str_invs[0];
  std::vector<BinaryFormula*> invs;
  BinaryFormula *base = NULL;
  std::vector<BinaryFormula*> assignments;
  for (std::string str : str_invs) {
    BinaryFormula *fm = new BinaryFormula(str);
    invs.push_back(fm);
  }

  for (BinaryFormula *fm : invs) {
    if (fm->GetOP() == "=" && !fm->IsRightVar()) {
      assignments.push_back(fm);
    } else {
      if (!base) {
        base = fm;
      }
    }
  }
  
  for (BinaryFormula *assign : assignments) {
    base->Update(assign->GetLHS(), assign->GetRHS());
  }

  std::string ret = base->ToString();
  for (BinaryFormula *fm : invs) {
    delete fm;
  }
  return ret;
}



/**
 * Generate main, support, makefile, scripts
 */
void Query::GenCode() {
  CodeGen generator;
  generator.SetFirstAST(m_new->GetASTNode()->GetAST());
  for (CFGNode *cfgnode : m_nodes) {
    generator.AddNode(cfgnode->GetASTNode());
  }
  generator.SetInput(m_inputs);
  generator.Compute();
  m_main = generator.GetMain();
  m_support = generator.GetSupport();
  m_makefile = generator.GetMakefile();
}


std::string Helium::derive_pre_cond(std::vector<std::string> str_invs, std::vector<std::string> str_trans) {
  std::string ret;
  // TODO select some, combine, get pre cond

  // std::string str_inv = merge_failure_condition(str_invs);
  // std::cout << "\t ********* " << str_inv  << "\n";
  // BinaryFormula *inv = new BinaryFormula(str_inv);

  // std::vector<BinaryFormula*> invs;
  // std::vector<BinaryFormula*> pres;
  std::vector<BinaryFormula*> trans;
  // create BinaryFormula here. CAUTION need to free them
  // for (std::string &s : str_invs) {
  //   invs.push_back(new BinaryFormula(s));
  // }
  for (std::string &s : str_trans) {
    trans.push_back(new BinaryFormula(s));
  }

  // TODO NOW Replace with failure condition in "init"
  // BinaryFormula *key_inv = get_key_inv(invs);
  BinaryFormula *key_inv = new BinaryFormula(m_failure_condition);
  // FIXME not guranteed
  if (!key_inv->IsLeftVar()) {
    key_inv->Inverse();
  }

  std::vector<BinaryFormula*> related_trans; // = get_related_trans(trans, vars);
  // for (BinaryFormula *bf : trans) {
  //   std::string item = bf->GetLHS();
  //   if (item == key_inv->GetLHS() || item == key_inv->GetRHS()) {
  //     related_trans.push_back(bf);
  //   }
  // }

  for (BinaryFormula *fm : trans) {
    std::cout << "using: " << fm->ToString()  << "\n";
    key_inv->Update(fm->GetLHS(), fm->GetRHS());
    std::cout << "Got: " << key_inv->ToString()  << "\n";
  }

  // input varaible name is special

  // for (BinaryFormula *tran : trans) {
  //   BinaryFormula *tmp = new BinaryFormula(*key_inv);
  //   if (tran->GetLHS() == tmp->GetLHS()) {
  //     tmp->UpdateLHS(tran->GetRHS());
  //   }
  //   if (tran->GetLHS() == tmp->GetRHS()) {
  //     tmp->UpdateRHS(tran->GetRHS());
  //   }


  //   // if contains no output variable, add it to ret
  //   std::string lhs = tmp->GetLHS();
  //   std::string rhs = tmp->GetRHS();
  //   if (!lhs.empty() && lhs[0] != 'O'
  //       && !rhs.empty() && rhs[0] != 'O') {
  //     ret.push_back(tmp->ToString());
  //   }
  //   delete tmp;
  // }
  // FIXME delete formulas
  ret = key_inv->ToString();
  delete key_inv;
  
  return ret;
}


/**
 * Initialize the query as the segment, into worklist
 * Traverse the worklist:
 * - Build the query
 * - Oracle judge for BS
 * - If bs, merge(BS, bs)
 * - else
 *   - Selector(q) into worklist
 * TODO workflow
 */
void Helium::process(ASTNode *node) {
  helium_print_trace("process");
  std::cout << "AST node created!"  << "\n";


  node->SetFailurePoint();

  Query *init_query = new Query(node);
  init(node);
  
  g_worklist.push_back(init_query);
  // init_query->Visualize();
  // init(node);
  while (!g_worklist.empty()) {
    // std::cout << "size of worklist: " << g_worklist.size()  << "\n";
    Query *query = g_worklist.front();
    g_worklist.pop_front();


    // reach the function definition, continue search, do not test, because will dont need, and will compile error
    // FIXME how to supply testing profile?
    if (query->New()->GetASTNode()->Kind() == ANK_Function) {
      std::vector<Query*> queries = select(query);
      g_worklist.insert(g_worklist.end(), queries.begin(), queries.end());
      continue;
    }
    // visulize Q?
    // query->Visualize();


    // query->Complete();

    query->ResolveInput();
    // std::set<CFGNode*> first_function_nodes = query->GetNodesForNewFunction();
    // std::vector<Variable> invs = get_input_variables(first_function_nodes);
    // std::string code = gen_code(query, invs);


    
    query->GenCode();
    Builder builder;
    builder.SetMain(query->GetMain());
    builder.SetSupport(query->GetSupport());
    builder.SetMakefile(query->GetMakefile());
    builder.Write();
    builder.Compile();
    std::cout << "Code Written to: " << builder.GetDir()  << "\n";
    if (builder.Success()) {
      utils::print("compile success\n", utils::CK_Green);
      std::string executable = builder.GetExecutable();
    } else {
      utils::print("compile error\n", utils::CK_Red);
      // query->Visualize();
      Query::MarkBad(query->New());
      query->Remove(query->New());
    }
    // std::string executable = write_and_compile(code);
    
    // TODO
    // query->GenTestSuite();
    // query->Test();

    CodeTester tester(builder.GetDir(), builder.GetExecutable(), query->GetInputs());
    // tester.SetInputs(query->GetInputs());
    // tester.SetExecutable(builder.GetDir(), builder.GetExecutable());
    // tester.GenTestSuite();
    tester.Test();



    CodeAnalyzer new_analyzer(builder.GetDir());
    new_analyzer.Compute();



    Analyzer analyzer(builder.GetDir() + "/io.csv", {});
    std::vector<std::string> invs = analyzer.GetInvariants();
    // std::vector<std::string> pres = analyzer.GetPreConditions();
    std::vector<std::string> trans = analyzer.GetTransferFunctions();
    if (HeliumOptions::Instance()->GetBool("print-analysis-result")) {
      std::cout << "== invariants"  << "\n";
      for (auto &s : invs) {
        std::cout << "\t" << s  << "\n";
      }
      // std::cout << "== pre condtions"  << "\n";
      // for (auto &s : pres) {
      //   std::cout << "\t" << s  << "\n";
      // }
      std::cout << "== transfer functions ------"  << "\n";
      for (auto &s : trans) {
        std::cout << "\t" << s  << "\n";
      }
    }

    std::string pre = derive_pre_cond(invs, trans);
    if (!pre.empty()) {
      if (pre_entry_point(pre)) {
        std::cout << "RESOLVED!!"  << "\n";
        exit(0);
      }
    }
    // if (!pres.empty()) {
    //   bool resolved = true;
    //   for (std::string pre : pres) {
    //     if (!pre_entry_point(pre)) {
    //       resolved = false;
    //       break;
    //     }
    //   }
    //   if (resolved) {
    //     std::cout << "RESOLVED!!!"  << "\n";
    //     // DEBUG here
    //     exit(0);
    //   }
    // }

    // TODO if the preconditions are related to only inputs, done!
    

    // tester.Analyze(res);
    // Profile profile = test(executable, input);
    // BugSig *bs = oracle(input, profile);
    // if (bs) {
    //   Merge(BS, bs);
    // } else {
      std::vector<Query*> queries = select(query);
      g_worklist.insert(g_worklist.end(), queries.begin(), queries.end());
      // std::cout << "---"  << "\n";
      // for (Query *q : queries) {
      //   q->Visualize();
      // }
    // }
  }
}

bool Helium::pre_entry_point(std::string pre) {
  BinaryFormula *formula = new BinaryFormula(pre);
  std::set<std::string> vars = formula->GetVars();
  for (std::string var : vars) {
    std::cout << "| " << var  << "\n";
    // this is argv:f!
    if (var.find(':') != std::string::npos) {
      var = var.substr(0, var.find(':'));
    }
    if (var != "argv" && var != "argc" && var != "optarg") {
      delete formula;
      return false;
    }
    if (var == "argv" || var == "argc") {
      // the argv should come from main
      // if (m_first->GetAST()->GetFunctionName() != "main") {
      //   return false;
      // }
    }
  }
  delete formula;
  return true;
}


/**
 * TODO context search
 * , Profile profile
 */
std::vector<Query*> Helium::select(Query *query) {
  helium_print_trace("select");
  std::vector<Query*> ret;
  // TODO hard to reach
  // if (profile.ReachFP()) {
  //   query->RemoveNew();
  // }

  // branch
  // TODO merge paths
  CFGNode *cfgnode = query->New();
  ASTNode *astnode = cfgnode->GetASTNode();
  if (astnode->Kind() == ANK_If) {
    std::set<Query*> queries = find_mergable_query(cfgnode, query);

    // std::cout << "For node: " << astnode->GetLabel()  << "\n";
    // std::cout << "mergable query no. " << queries.size()  << "\n";
    
    if (!queries.empty()) {
      for (Query *q : queries) {
        // modify in place
        // std::cout << "merging: "  << q->GetNodes().size() << "\n";
        q->Merge(query);
        // std::cout << "After merge: " << q->GetNodes().size()  << "\n";
      }
      return ret;
    } else {
      // add it self to the waiting list
      g_waiting_quries[cfgnode].insert(query);
    }
  }
  // Now, the query should not be mergeable to reach here
  // predecessor
  CFG *cfg = Resource::Instance()->GetCFG(astnode->GetAST());
  if (!cfg) return ret;
  std::set<CFGNode*> preds = cfg->GetPredecessors(cfgnode);
  for (CFGNode *pred : preds) {
    // FIXME this will not work on loops
    if (query->ContainNode(pred)) continue;
    // continue here, easier..
    if (Query::IsBad(pred)) continue;
    Query *q = new Query(*query);
    q->Add(pred);
    ret.push_back(q);
  }

  // update g_propagating_queries
  g_propagating_queries[query].insert(ret.begin(), ret.end());
  return ret;
}


std::set<Query*> Helium::find_mergable_query(CFGNode *node, Query *orig_query) {
  helium_print_trace("find_mergable_query");
  std::set<Query*> ret;
  if (g_waiting_quries.count(node) == 0) {
    return ret;
  }
  std::set<Query*> queries = g_waiting_quries[node];
  // follow propagation path for the most recent position
  std::set<Query*> worklist;
  worklist.insert(queries.begin(), queries.end());
  std::set<Query*> candidates;
  while (!worklist.empty()) {
    Query *q = *worklist.begin();
    worklist.erase(q);
    if (g_propagating_queries.count(q) == 1) {
      worklist.insert(g_propagating_queries[q].begin(), g_propagating_queries[q].end());
    } else {
      candidates.insert(q);
    }
  }
  // for all the candidates that have same transfer function as orig_query
  for (Query *q : candidates) {
    // use random here
    // if (utils::rand_bool()) {
    //   ret.insert(q);
    // }
    ret.insert(q);
  }
  return ret;
}








































#if 0
int Helium::countFunction() {
  int ret=0;
  for (std::string file : m_files) {
    XMLDoc doc;
    utils::file2xml(file, doc);
    XMLNodeList nodes = find_nodes(doc, NK_Function);
    ret += nodes.size();
  }
  return ret;
}

void
Helium::Run() {
  /**
   * Print some meta data for the benchmark project.
   */
  std::cerr << "***** Helium Benchmark Meta *****"  << "\n";
  std::cerr << "** total file: " << m_files.size()  << "\n";
  // int func_count = countFunction();
  // std::cerr << "** totla function: " << func_count  << "\n";
  std::cerr << "*********************************" << "\n";
  // ExpASTDump::Instance()->file_count = m_files.size();
  // ExpASTDump::Instance()->func_count = func_count;
  // ExpASTDump::Instance()->benchmark = m_folder;
  // double t1 = utils::get_time();

  // double t2 = utils::get_time();
  std::cerr << "********** End of Helium **********"  << "\n";
  if (PrintOption::Instance()->Has(POK_BuildRate)) {
    std::cerr << "Compile Success Count: " << g_compile_success_no  << "\n";
    std::cerr << "Compile Error Count: " << g_compile_error_no  << "\n";
    std::cerr << "Buildrate: " << (double)g_compile_success_no / (double)(g_compile_success_no + g_compile_error_no)  << "\n";
  }
  // ExpASTDump::Instance()->time = t2 - t1;

  /**
   * Now its time to dump the *Dump clases
   */
  // std::cout << "====DUMP START========="  << "\n";
  // std::cout << ExpASTDump::Instance()->GetHeader()  << "\n";
  // std::cout << ExpASTDump::Instance()->dump() << "\n";
  // std::cout << "====DUMP STOP========="  << "\n";

  /**
   * Also, store one version to the file.
   */
  // utils::append_file("dump_out.txt", ExpASTDump::Instance()->dump() + "\n");
  // ExpASTDump::Instance()->AppendData();
  // std::cout << BuildRatePlotDump::Instance()->dump()  << "\n";
}











/**
 * Deprecated code
 */












/**
 * Get segments based on config file.
 * this should be a config file with the filename:line_number format
 * the detailed formats:
 * a.c:504
 * TODO a.c:504-510
 * TODO to support function name
 * @return a.c -> 102,208
 */
// std::map<std::string, std::vector<int> > parse_selection_conf(const std::string& file) {
//   std::map<std::string, std::vector<int> > result;
//   std::string content = utils::read_file(file);
//   // delimiter by *space*
//   std::vector<std::string> lines = utils::split(content);
//   for (std::string &line : lines) {
//     std::vector<std::string> tmp = utils::split(line, ':');
//     assert(tmp.size() == 2 && "Config file error for code selection.");
//     std::string f = tmp[0];
//     std::string l_str = tmp[1];
//     utils::trim(f);
//     utils::trim(l_str);
//     int l = atoi(l_str.c_str());
//     if (result.find(f) == result.end()) {
//       result[f] = std::vector<int>();
//     }
//     // FIXME value or reference?
//     result[f].push_back(l);
//   }
//   return result;
// }

// TEST(helium_test_case, parse_selection_conf_test) {
//   char tmp_dir[] = "/tmp/helium-test-temp.XXXXXX";
//   char *result = mkdtemp(tmp_dir);
//   ASSERT_TRUE(result != NULL);
//   std::string dir = tmp_dir;
//   std::string filename = dir+"/a.c";
//   const char *code = R"prefix(

// a.c:100
// a.c:235
// sub/dir/b.cpp:364

// )prefix";
//   utils::write_file(filename, code);

//   std::map<std::string, std::vector<int> > m = parse_selection_conf(filename);
//   ASSERT_EQ(m.size(), 2);
//   std::vector<int> a = m["a.c"];
//   ASSERT_EQ(a.size(), 2);
//   EXPECT_EQ(a[0], 100);
//   EXPECT_EQ(a[1], 235);
//   std::vector<int> b = m["sub/dir/b.cpp"];
//   ASSERT_EQ(b.size(), 1);
//   EXPECT_EQ(b[0], 364);
// }

#endif
