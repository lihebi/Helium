#include "helium/workflow/helium.h"
#include <string>
#include <iostream>

#include "helium/workflow/helium_utils.h"

#include "helium/utils/helium_options.h"
#include "helium/parser/xml_doc_reader.h"
#include "helium/parser/xmlnode.h"
#include "helium/parser/ast_node.h"
#include "helium/parser/cfg.h"

#include "helium/utils/utils.h"
#include "helium/utils/dump.h"

#include "helium/resolver/snippet.h"
#include "helium/resolver/resolver.h"
#include "helium/resolver/snippet_db.h"
#include "helium/parser/resource.h"
#include "helium/utils/log.h"
#include "helium/workflow/builder.h"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include "helium/utils/helper.h"
#include "helium/workflow/tester.h"
#include "helium/workflow/analyzer.h"

#include "helium/parser/cond.h"

namespace fs = boost::filesystem;


using namespace utils;




Helium::Helium(PointOfInterest *poi) {
  std::cout << "Starting Helium on point of interest: " << poi->GetPath() << ":" << poi->GetLinum() << "\n";
  assert(poi);
  m_poi = poi;

  XMLDoc *doc = XMLDocReader::Instance()->ReadFile(poi->GetPath());
  int linum = get_true_linum(poi->GetPath(), poi->GetLinum());
  std::cout << "Converted linum after preprocessing: " << linum << "\n";

  XMLNode xmlnode;
  if (poi->GetType() == "stmt") {
    xmlnode = find_node_on_line(doc->document_element(),
                                     {NK_Stmt, NK_ExprStmt, NK_DeclStmt,
                                         NK_Return, NK_Break, NK_Continue},
                                     linum);
  } else if (poi->GetType() == "loop") {
    xmlnode = find_node_on_line(doc->document_element(),
                                     {NK_Do, NK_While, NK_For}, linum);
  } else {
    std::cerr << "Only Support Stmt and Loop as POI." << "\n";
  }
  if (!xmlnode) {
    std::cerr << "EE: Cannot find SrcML node based on POI." << "\n";
    m_status=HS_POIInvalid;
    // exit(1);
    return;
  }

  XMLNode func = get_function_node(xmlnode);
  if (!func) {
    std::cerr << "EE: Cannot find function node for POI." << "\n";
    m_status=HS_POIInvalid;
    return;
  }
  std::string func_name = function_get_name(func);
  int func_linum = get_node_line(func);
  std::set<int> ids = SnippetDB::Instance()->LookUp(func_name, {SK_Function});
  if (ids.empty()) {
    std::cerr << "EE: Cannot find the function " << func_name << " enclosing POI." << "\n";
    // exit(1);
    m_status=HS_POIInvalid;
    return;
  }

  for (int id : ids) {
    // std::cout << id << "\n";
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
          // find the node by linum
          // TODO if this is a loop, how? Use all nodes inside the loop?
          ASTNode *target = ast->GetNodeByLinum(target_linum);
          if (target) {
            CFG *cfg = Resource::Instance()->GetCFG(target->GetAST());
            CFGNode *target_cfgnode = cfg->ASTNodeToCFGNode(target);
            // (HEBI: Set Failure Point)
            target->SetFailurePoint();
            Segment::SetPOI(target_cfgnode);

            m_failure_condition = m_poi->GetFailureCondition();

            // construct the initial query by the initial nodes
            // Let's use the entire loop at POI
            Segment *init_query = new Segment(target);
            m_worklist.push_back(init_query);
          }
        }
      }
    }
  }
  // initial query:
  if (m_worklist.empty()) {
    std::cerr << "Cannot construct the initial query." << "\n";
    m_status=HS_POIInvalid;
    // exit(1);
    return;
  }
  m_status=HS_Success;
  std::cout << "Initial query: " << m_worklist.size() << "\n";
  process();
}

/**
 * Debug the remove algorithm. This should not appear in production code
 */
void Helium::debugRemoveAlg(Segment *segment) {
  // print out the segment information
  segment->Dump();
      
  Analyzer *old_profile = segment->GetProfile();
  // seg + n
  std::cout << "seg+n" << "\n";
  Analyzer *profile1 = testProfile(segment);
  // seg - mark + n
  std::cout << "seg-mark+n" << "\n";
  segment->ActivateRemoveMark();
  Analyzer *profile2 = testProfile(segment);
  segment->RestoreRemoveMark();

  // outputing the transfer used in all three profiles
  std::cout << utils::CYAN << "Used Trans" << "\n";
  Analyzer::print_used_trans(old_profile);
  std::cout << "---" << "\n";
  Analyzer::print_used_trans(profile1);
  std::cout << "---" << "\n";
  Analyzer::print_used_trans(profile2);
  std::cout << utils::RESET << "\n";
      
  if (Analyzer::same_trans(profile1, profile2)) {
    helium_log("same");
    // does not report
    // we need to continue based on whether it is equal to the old profile
    // mark as remove if does not chagne
    if (Analyzer::same_trans(profile1, old_profile)) {
      helium_log("123");
      // std::cout << utils::RED << "Marking remove" << utils::RESET << "\n";
      segment->MarkRemove(segment->New());
      // std::cout << segment->New().size() << "\n";
    } else {
      helium_log("12,3");
      // if it does change, it makes sense to report it.
      // But actually we don't need to add any mark
      std::cout
        << "The context transfer profile changed,"
        << "but both mark and non-mark version change to the same"
        << "\n";
    }
    // record the new profile.
    // Since profile1 and profile2 are "same" in terms of variable of interest,
    // we choose one of them
    segment->SetProfile(profile1);
    // we finally need to continue search
    std::vector<Segment*> queries = select(segment);
    m_worklist.insert(m_worklist.end(), queries.begin(), queries.end());
  } else {
    // report
    std::cout << utils::GREEN << "Got the difference!" << utils::RESET << "\n";
    // now we can skip this segment (does not context search)
    helium_log("difference");
    if (Analyzer::same_trans(old_profile, profile1)) {
      helium_log("13,2");
    } else if (Analyzer::same_trans(old_profile, profile2)) {
      helium_log("23,1");
    } else {
      helium_log("1,2,3");
    }
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
  // helium_print_trace("process");
  std::cout << "Helium Processing ..." << "\n";
  int seg_limit = HeliumOptions::Instance()->GetInt("segment-per-poi-limit");
  int compile_error_limit = HeliumOptions::Instance()->GetInt("compile-error-limit-per-poi");
  int seg_ct=0;
  int compile_error_ct=0;
  while (!m_worklist.empty()) {
    // std::cout << "size of worklist: " << m_worklist.size()  << "\n";
    // get the segment out from worklist
    Segment *segment = m_worklist.front();

    // checking of limits
    seg_ct++;
    if (seg_limit>0 && seg_ct>seg_limit) {
      std::cerr << "Reach segment-per-poi limit. Returning." << "\n";
      return;
    }
    if (compile_error_limit > 0 && compile_error_limit < compile_error_ct) {
      std::cerr << "Reach compile-error-limit-per-poi limit. Returning" << "\n";
      return;
    }

    m_worklist.pop_front();
    if (!segment->IsValid()) {
      std::cout << "Segment Invalid due to removing of callsite." << "\n";
      continue;
    }
    std::string label = segment->Head()->GetLabel();
    utils::trim(label);
    if (label.size() > 20) {
      label = label.substr(0,20);
      label += "...";
    }
    std::cout << "Processing query with the head node: "
              << CYAN << label << RED << " "
              << m_worklist.size() << RESET << " remaining in worklist.""\n" ;

    // reach the function definition, continue search,
    // do not test, because will dont need, and will compile error
    if (segment->Head()->GetASTNode()->Kind() == ANK_Function) {
      std::vector<Segment*> queries = select(segment);
      m_worklist.insert(m_worklist.end(), queries.begin(), queries.end());
      continue;
    }


    // process teh segment and generate code
    segment->PatchGrammar();
    
    segment->ResolveInput();
    segment->GenCode();

    // write file and compile
    Builder builder;
    builder.SetMain(segment->GetMain());
    builder.SetSupport(segment->GetSupport());
    builder.SetMakefile(segment->GetMakefile());

    builder.Write();
    builder.Compile();
    if (HeliumOptions::Instance()->GetBool("print-code-output-location")) {
      std::cout << "\t" << "Code Written to: " << builder.GetDir()  << "\n";
    }
    if (HeliumOptions::Instance()->GetBool("print-compile-info")) {
      std::cout << "\t" << "Compile: " << (builder.Success() ? "true" : "false") << "\n";
    }

    // if compile error, remove the new statement
    if (!builder.Success()) {
      compile_error_ct++;
      std::cerr << utils::RED << "compile error"<< utils::RESET << "\n";
      if (HeliumOptions::Instance()->GetBool("pause-compile-error")) {
        std::cout << "Paused, press enter to continue ..." << std::flush;
        getchar();
      }
      // std::cout << utils::RED << "Removing the new node" << "\n";
      segment->Remove(segment->New());
      // remove the new one that cause compile error
      // But we still want to continue propagate for it
      // this selection will not have profile data
      // (HEBI: Another place to propagate query)
      std::vector<Segment*> queries = select(segment);
      m_worklist.insert(m_worklist.end(), queries.begin(), queries.end());
      // std::cout << "The segment is valid? " << segment->IsValid() << "\n";
      // std::cout << utils::RESET << "\n";
      // m_worklist.push_back(segment);
      continue;
    }
    std::cerr << utils::GREEN << "compile success" << utils::RESET << "\n";
    std::string executable = builder.GetExecutable();


    /********************************
     * debug removing algorithm
     *******************************/
    // This is a self contained exp, the code after this will be skipped. Need refactoring.
    if (HeliumOptions::Instance()->GetBool("debug-remove-alg")) {
      debugRemoveAlg(segment);
      continue;
    }

    // run test
    if (HeliumOptions::Instance()->GetBool("run-test")) {
      // run test
      Tester tester(builder.GetDir(), builder.GetExecutableName(), segment->GetInputs());
      tester.Test();

      // get result
      if (fs::exists(builder.GetDir() + "/result.txt")) {
        Analyzer *analyzer = new Analyzer(builder.GetDir());
        analyzer->GetCSV();
        analyzer->AnalyzeCSV();

        // resolve the query

        // TODO this should be better organized instead of resolver2
        if (HeliumOptions::Instance()->GetBool("use-query-resolver-2")) {
          // resolve query 2 is used for assertion experiment
          bool res = analyzer->ResolveQuery2(m_failure_condition);
          if (res) {
            return;
          }
        }

        // ordinary resolver
        analyzer->ResolveQuery(m_failure_condition);



        // aggressive remove
        
        // m_segment_profiles[segment] = analyzer;
        if (segment->GetProfile()) {
          // before setting the profile, check if the new profile is the same as previous one?
          // if so, remove the new branch and do context search.
          // this is aggressive-remove option in config file
          if (HeliumOptions::Instance()->GetBool("aggressive-remove")) {
            Analyzer *old_profile = segment->GetProfile();
            // same transfer function, we are going to do something
            if (Analyzer::same_trans(old_profile, analyzer)) {
              segment->Remove(segment->New());
              // (HEBI: aggressive remove)
              std::vector<Segment*> queries = select(segment);
              m_worklist.insert(m_worklist.end(), queries.begin(), queries.end());
              continue;
            }
          }
        }


        // set profile
        segment->SetProfile(analyzer);

        // std::cout << utils::GREEN << "\n";
        // std::cout << "Used transfer functions:" << "\n";
        // std::map<std::string, std::string> mm = analyzer->GetUsedTransfer();
        // for (auto m : mm) {
        //   std::cout << m.first << " ==> " << m.second << "\n";
        // }
        // std::cout << utils::RESET << "\n";
        
        // (HEBI: remove branch if not covered)
        if (!analyzer->IsCovered()) {
          std::cout << utils::RED << "POI is not covered" << utils::RESET << "\n";
          // DEBUG remove branch or not
          if (HeliumOptions::Instance()->GetBool("remove-branch-if-not-covered")) {
            segment->RemoveNewBranch();
            m_worklist.push_back(segment);
          } else {
            std::vector<Segment*> queries = select(segment);
            m_worklist.insert(m_worklist.end(), queries.begin(), queries.end());
          }
        } else {
          std::cout << utils::GREEN << "POI is covered" << utils::RESET << "\n";
          // (HEBI: propagating the query, the normal place)
          // Now I want to remove node if not covered
          // FIXME this might cause infinite loop because the criteria of not following back edge is by checking whether the node is in current selection
          // TODO Also, I want to make a helium option that control how to generally do context search: I want to flavor the path going up instead of following loop backedge
          std::vector<Segment*> queries = select(segment);
          m_worklist.insert(m_worklist.end(), queries.begin(), queries.end());
        }

        if (analyzer->IsBugTriggered()) {
          std::cout << utils::GREEN << "Bug is triggered." << utils::RESET << "\n";
        } else {
          std::cout << utils::RED << "Bug is not triggered." << utils::RESET << "\n";
        }
      }
    } else {
      // didn't run test ...
      // but still do context search, for build rate test
      std::vector<Segment*> queries = select(segment);
      m_worklist.insert(m_worklist.end(), queries.begin(), queries.end());
    }
  }
}


/**
 * gen code, compile, test, and analyze the segment
 * @return analyzer as profile
 */
Analyzer* Helium::testProfile(Segment *segment) {
  segment->PatchGrammar();
  segment->ResolveInput();
  segment->GenCode();
  Builder builder;
  builder.SetMain(segment->GetMain());
  builder.SetSupport(segment->GetSupport());
  builder.SetMakefile(segment->GetMakefile());
  builder.Write();
  builder.Compile();
  std::cout << "\t" << "Code Written to: " << builder.GetDir()  << "\n";
  if (builder.Success()) {
    std::cout << "testProfile compile Success" << "\n";
    Tester tester(builder.GetDir(), builder.GetExecutableName(), segment->GetInputs());
    tester.Test();
    if (fs::exists(builder.GetDir() + "/result.txt")) {
      Analyzer *analyzer = new Analyzer(builder.GetDir());
      analyzer->GetCSV();
      analyzer->AnalyzeCSV();
      analyzer->ResolveQuery(m_failure_condition);
      // This new analyzer is the profile
      return analyzer;
    }
  } else {
    std::cout << "testProfile compile Error" << "\n";
  }
  return NULL;
}



/**
 * context search
 */
std::vector<Segment*> Helium::select(Segment *query) {
  helium_print_trace("select");
  std::vector<Segment*> ret;
  // branch
  // TODO merge paths
  CFGNode *cfgnode = query->Head();
  ASTNode *astnode = cfgnode->GetASTNode();
  if (astnode->Kind() == ANK_If) {
    std::set<Segment*> queries = find_mergable_query(cfgnode, query);
    if (!queries.empty()) {
      for (Segment *q : queries) {
        // modify in place
        q->Merge(query);
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
      // DEBUG FIXME FIXME
      // if (query->ContainNode(pred)) continue;
      if (query->ContainNode(pred)) {
        // if it is not a branch, we remove it
        if (!pred->IsBranch()) {
          continue;
        }
      }
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


/**
 * Merge query
 */
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


    std::string merge_method = HeliumOptions::Instance()->GetString("merge-method");
    if (merge_method == "transfer") {
      if (sameTransfer(orig_query, q)) {
        // std::cout << utils::CYAN
        //           << "Transfer function the same, merging .."
        //           << utils::RESET << "\n";
        ret.insert(q);
      } else {
        // std::cout << utils::CYAN << "Not same, cannot merge .." << utils::RESET << "\n";
      }
    } else if (merge_method == "transfer-all") {
      if (sameAllTransfer(orig_query, q)) {
        ret.insert(q);
      }
    } else if (merge_method == "aggressive") {
      ret.insert(q);
    } else if (merge_method == "random") {
      if (utils::rand_bool()) {
        ret.insert(q);
      }
    } else if (merge_method == "no") {
      return ret;
    }
  }
  return ret;
}

bool Helium::sameTransfer(Segment *s1, Segment *s2) {
  if (!s1->GetProfile() && !s2->GetProfile()) return true;
  if (s1->GetProfile() && s2->GetProfile()) {
    return Analyzer::same_trans(s1->GetProfile(), s2->GetProfile());
  }
  return false;
}

/**
 * whether all the transfers are same
 */
bool Helium::sameAllTransfer(Segment *s1, Segment *s2) {
  if (!s1->GetProfile() && !s2->GetProfile()) return true;
  if (s1->GetProfile() && s2->GetProfile()) {
    return Analyzer::same_trans_2(s1->GetProfile(), s2->GetProfile());
  }
  return false;
}
