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
#include "parser/resource.h"
#include "utils/log.h"
#include "builder.h"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include "helper.h"
#include "tester.h"

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
              Segment *init_query = new Segment(target);
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
    Segment *query = m_worklist.front();
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
      std::vector<Segment*> queries = select(query);
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
        Tester tester(builder.GetDir(), builder.GetExecutableName(), query->GetInputs());
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
        Segment::MarkBad(query->New());
        query->Remove(query->New());
      }
    }

    // TODO get test profile
    // TODO oracle for bug signature
    // TODO merge bug signature
    std::vector<Segment*> queries = select(query);
    m_worklist.insert(m_worklist.end(), queries.begin(), queries.end());
  }
}



/**
 * context search
 */
std::vector<Segment*> Helium::select(Segment *query) {
  helium_print_trace("select");
  std::vector<Segment*> ret;
  // TODO hard to reach
  // if (profile.ReachFP()) {
  //   query->RemoveNew();
  // }

  // branch
  // TODO merge paths
  CFGNode *cfgnode = query->New();
  ASTNode *astnode = cfgnode->GetASTNode();
  if (astnode->Kind() == ANK_If) {
    std::set<Segment*> queries = find_mergable_query(cfgnode, query);

    // std::cout << "For node: " << astnode->GetLabel()  << "\n";
    // std::cout << "mergable query no. " << queries.size()  << "\n";
    
    if (!queries.empty()) {
      for (Segment *q : queries) {
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
      // if (Segment::IsBad(pred)) continue;
      Segment *q = new Segment(*query);
      q->Add(pred);
      ret.push_back(q);
    }
  } else {
    // inter procedure
    preds = cfg->GetInterPredecessors(cfgnode);
    for (CFGNode *pred : preds) {
      if (query->ContainNode(pred)) continue;
      Segment *q = new Segment(*query);
      // add and MARK as inter procedure
      q->Add(pred, true);
      ret.push_back(q);
    }
  }

  // update g_propagating_queries
  m_propagating_queries[query].insert(ret.begin(), ret.end());
  return ret;
}


std::set<Segment*> Helium::find_mergable_query(CFGNode *node, Segment *orig_query) {
  helium_print_trace("find_mergable_query");
  std::set<Segment*> ret;
  if (m_waiting_quries.count(node) == 0) {
    return ret;
  }
  std::set<Segment*> queries = m_waiting_quries[node];
  // follow propagation path for the most recent position
  std::set<Segment*> worklist;
  worklist.insert(queries.begin(), queries.end());
  std::set<Segment*> candidates;
  while (!worklist.empty()) {
    Segment *q = *worklist.begin();
    worklist.erase(q);
    if (m_propagating_queries.count(q) == 1) {
      worklist.insert(m_propagating_queries[q].begin(), m_propagating_queries[q].end());
    } else {
      candidates.insert(q);
    }
  }
  // for all the candidates that have same transfer function as orig_query
  for (Segment *q : candidates) {
    // use random here
    // if (utils::rand_bool()) {
    //   ret.insert(q);
    // }
    ret.insert(q);
  }
  return ret;
}
