#include "helium.h"
#include <string>
#include <iostream>

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
#include "analyzer.h"

#include "builder.h"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>


#include "code_test.h"
#include "code_gen.h"
#include "code_analyze.h"
#include "helper.h"

namespace fs = boost::filesystem;


using namespace utils;



Helium::Helium(PointOfInterest *poi) {
  std::cout << "Starting Helium on point of interest: " << poi->GetPath() << ":" << poi->GetLinum() << "\n";

  XMLDoc *doc = XMLDocReader::Instance()->ReadFile(poi->GetPath());
  int linum = get_true_linum(poi->GetPath(), poi->GetLinum());
  std::cout << "Converted linum after preprocessing: " << linum << "\n";
  
  if (poi->GetType() == "stmt") {
    XMLNode node = find_node_on_line(doc->document_element(),
                                     {NK_Stmt, NK_ExprStmt, NK_DeclStmt,
                                         NK_Return, NK_Break, NK_Continue},
                                     linum);
    if (!node) {
      std::cerr << "EE: Cannot find SrcML node based on POI." << "\n";
      exit(1);
    }
    XMLNode func = get_function_node(node);
    std::string func_name = function_get_name(func);
    int func_linum = get_node_line(func);
    std::set<int> ids = SnippetDB::Instance()->LookUp(func_name, {SK_Function});
    for (int id : ids) {
      SnippetMeta meta = SnippetDB::Instance()->GetMeta(id);
      std::string filename = fs::path(meta.filename).filename().string();
      if (filename == poi->GetFilename()) {
        // construct
        AST *ast = Resource::Instance()->GetAST(id);
        if (ast) {
          ASTNode *root = ast->GetRoot();
          if (root) {
            int ast_linum = root->GetBeginLinum();
            int target_linum = linum - func_linum + ast_linum;
            ASTNode *target = ast->GetNodeByLinum(target_linum);
            if (target) {
              target->SetFailurePoint();
              Query *init_query = new Query(target);
              m_worklist.push_back(init_query);
            }
          }
        }
      }
    }
    if (m_worklist.empty()) {
      std::cerr << "Cannot construct the initial query." << "\n";
      exit(1);
    }
    process();
  }
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
void Helium::process() {
  helium_print_trace("process");
  while (!m_worklist.empty()) {
    // std::cout << "size of worklist: " << m_worklist.size()  << "\n";
    Query *query = m_worklist.front();
    m_worklist.pop_front();
    std::string label = query->New()->GetLabel();
    if (label.size() > 10) {
      label = label.substr(0,10);
      label += "...";
    }
    std::cout << CYAN << "Processing query with the new node: "
              << label << "."
              << m_worklist.size() << " remaining in worklist.""\n"
              << RESET;

    // reach the function definition, continue search, do not test, because will dont need, and will compile error
    // FIXME how to supply testing profile?
    if (query->New()->GetASTNode()->Kind() == ANK_Function) {
      std::vector<Query*> queries = select(query);
      m_worklist.insert(m_worklist.end(), queries.begin(), queries.end());
      continue;
    }
    query->ResolveInput();
    query->GenCode();
    
    Builder builder;
    builder.SetMain(query->GetMain());
    builder.SetSupport(query->GetSupport());
    builder.SetMakefile(query->GetMakefile());
    builder.Write();
    builder.Compile();
    if (HeliumOptions::Instance()->GetBool("print-code-output-location")) {
      std::cout << "\t" << "Code Written to: " << builder.GetDir()  << "\n";
    }
    if (HeliumOptions::Instance()->GetBool("print-compile-info")) {
      std::cout << "\t" << "Compile: " << (builder.Success() ? "true" : "false") << "\n";
    }
    if (builder.Success()) {
      // utils::print("compile success\n", utils::CK_Green);

      std::cerr << utils::GREEN << "compile success" << utils::RESET << "\n";
      
      std::string executable = builder.GetExecutable();
      if (HeliumOptions::Instance()->GetBool("run-test")) {
        CodeTester tester(builder.GetDir(), builder.GetExecutableName(), query->GetInputs());
        tester.Test();
        // CodeAnalyzer new_analyzer(builder.GetDir());
        // new_analyzer.Compute();

        // Analyzer analyzer(builder.GetDir() + "/io.csv", {});
        // std::vector<std::string> invs = analyzer.GetInvariants();
        // // std::vector<std::string> pres = analyzer.GetPreConditions();
        // std::vector<std::string> trans = analyzer.GetTransferFunctions();
        // if (HeliumOptions::Instance()->GetBool("print-analysis-result")) {
        //   std::cout << "== invariants"  << "\n";
        //   for (auto &s : invs) {
        //     std::cout << "\t" << s  << "\n";
        //   }
        //   // std::cout << "== pre condtions"  << "\n";
        //   // for (auto &s : pres) {
        //   //   std::cout << "\t" << s  << "\n";
        //   // }
        //   std::cout << "== transfer functions ------"  << "\n";
        //   for (auto &s : trans) {
        //     std::cout << "\t" << s  << "\n";
        //   }
        // }

        // std::string pre = derive_pre_cond(invs, trans);
        // if (!pre.empty()) {
        //   if (pre_entry_point(pre)) {
        //     std::cout << "RESOLVED!!"  << "\n";
        //     exit(0);
        //   }
        // }
      }
    } else {
      std::cerr << utils::RED << "compile error"<< utils::RESET << "\n";
      // query->Visualize();
      // compile error, skip this statement, and mark it as bad
      // however, if the "New" node is from interprocedure, we cannot remove it, but stop this line
      // That is because we need this callsite after all
      if (query->IsInter()) {
        std::cerr << "Inter procedure search, but the callsite is not compilible, stop this query." << "\n";
        continue;
      } else {
        Query::MarkBad(query->New());
        query->Remove(query->New());
      }
    }

    // TODO get test profile
    // TODO oracle for bug signature
    // TODO merge bug signature
    std::vector<Query*> queries = select(query);
    m_worklist.insert(m_worklist.end(), queries.begin(), queries.end());
  }
}



/**
 * context search
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
      m_waiting_quries[cfgnode].insert(query);
    }
  }
  // Now, the query should not be mergeable to reach here
  // predecessor
  CFG *cfg = Resource::Instance()->GetCFG(astnode->GetAST());
  if (!cfg) return ret;
  // FIXME if the predecessor is interprocedure, and the compile failed,
  // should not continue this line because the callsite has to be removed.
  std::set<CFGNode*> preds = cfg->GetPredecessors(cfgnode);
  if (!preds.empty()) {
    for (CFGNode *pred : preds) {
      // FIXME this will not work on loops
      if (query->ContainNode(pred)) continue;
      // continue here, easier..
      // UPDATE I don't really need to check this here, the compilation will tell this is bad.
      // if (Query::IsBad(pred)) continue;
      Query *q = new Query(*query);
      q->Add(pred);
      ret.push_back(q);
    }
  } else {
    // inter procedure
    preds = cfg->GetInterPredecessors(cfgnode);
    for (CFGNode *pred : preds) {
      if (query->ContainNode(pred)) continue;
      Query *q = new Query(*query);
      // add and MARK as inter procedure
      q->Add(pred, true);
      ret.push_back(q);
    }
  }

  // update g_propagating_queries
  m_propagating_queries[query].insert(ret.begin(), ret.end());
  return ret;
}


std::set<Query*> Helium::find_mergable_query(CFGNode *node, Query *orig_query) {
  helium_print_trace("find_mergable_query");
  std::set<Query*> ret;
  if (m_waiting_quries.count(node) == 0) {
    return ret;
  }
  std::set<Query*> queries = m_waiting_quries[node];
  // follow propagation path for the most recent position
  std::set<Query*> worklist;
  worklist.insert(queries.begin(), queries.end());
  std::set<Query*> candidates;
  while (!worklist.empty()) {
    Query *q = *worklist.begin();
    worklist.erase(q);
    if (m_propagating_queries.count(q) == 1) {
      worklist.insert(m_propagating_queries[q].begin(), m_propagating_queries[q].end());
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

#endif


