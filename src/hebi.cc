#include "hebi.h"
#include "parser/xml_doc_reader.h"
#include "utils/log.h"
#include "workflow/resource.h"

#include "common.h"
#include "config/options.h"

#include <iostream>

void hebi(std::string filename, POISpec poi) {
  print_trace("hebi");
  XMLDoc *doc = XMLDocReader::Instance()->ReadFile(filename);
  int linum = get_true_linum(filename, poi.linum);
  std::cout << "true line number: " <<  linum  << "\n";
  if (poi.type == "stmt") {
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


// ASTNode *g_fp = NULL;
// std::vector<Variable> g_outvs;
// Formula g_f_cond;
std::deque<Query*> worklist;


/**
 * 
 * Build a single segment to get the error condition.
 * - Build the query
 * - GenerateInput
 * - Test
 * - Oracle infer failure condition
 */
void init(ASTNode *node) {
  // Query *query = new Query(node);
  // AST *ast = query->GetNewAST();
  // std::set<ASTNode*> first_ast_nodes = query->GetNodes(ast);
  // std::vector<Variable> invs = get_input_variables(first_ast_nodes);
  // std::vector<Variable> outvs = get_output_variables(node);
  // std::string code = gen_code(query, invs, outvs);
  // std::string executable = write_and_compile(code);
  // InputSpec input = generate_input(invs);
  // Profile profile = test(executable, input);
  // Formula f_cond = gen_failure_cond(input, profile);

  // g_fp = node;
  // g_outvs = outvs;
  // g_f_cond = f_cond;
  // worklist.push_back(query);
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
void process(ASTNode *node) {
  print_trace("process");
  std::cout << "AST node created!"  << "\n";
  // init(node);
  while (!worklist.empty()) {
    Query *query = worklist.front();
    worklist.pop_front();
    // std::set<ASTNode*> first_ast_nodes = query->GetNodes(ast);
    // std::vector<Variable> invs = get_input_variables(first_ast_nodes);
    // std::string code = gen_code(query, invs, outvs);
    // std::string executable = write_and_compile(code);
    // InputSpec input = generate_input(invs);
    // Profile profile = test(executable, input);
    // BugSig *bs = oracle(input, profile);
    // if (bs) {
    //   Merge(BS, bs);
    // } else {
      std::vector<Query*> queries = select(query);
      worklist.insert(worklist.end(), queries.begin(), queries.end());
    // }
  }
}

std::map<ASTNode*, std::set<Query*> > g_waiting_quries;
std::map<Query*, std::set<Query*> > g_propagating_queries;

/**
 * TODO context search
 * , Profile profile
 */
std::vector<Query*> select(Query *query) {
  std::vector<Query*> ret;
  // TODO hard to reach
  // if (profile.ReachFP()) {
  //   query->RemoveNew();
  // }

  // branch
  // TODO merge paths
  ASTNode *node = query->New();
  if (node->Kind() == ANK_If) {
    std::set<Query*> queries = find_mergable_query(node, query);
    if (!queries.empty()) {
      for (Query *q : queries) {
        // modify in place
        q->Merge(query);
      }
      return ret;
    }
  }
  // Now, the query should not be mergeable to reach here
  // predecessor
  CFG *cfg = Resource::Instance()->GetCFG(node->GetAST());
  std::set<ASTNode*> preds = cfg->GetPredecessors(node);
  for (ASTNode *pred : preds) {
    Query *q = new Query(*query);
    q->Add(pred);
    ret.push_back(q);
  }
  return ret;
}


std::set<Query*> find_mergable_query(ASTNode *node, Query *orig_query) {
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
    if (utils::rand_bool()) {
      ret.insert(q);
    }
  }
  return ret;
}


/**
 * This is the minimize list. This is global. It can also be a query local list.
 */
std::set<ASTNode*> g_m_list;

/**
 * @param g_f_cond
 * TODO oracle
 * TODO entry point
 * TODO z3
 */
// BugSig *oracle(InputSpec *input, Profile profile) {
//   trans = infer_trans(profile);
//   pre_cond = solver(g_f_cond, trans);
//   if (resolved(pre_cond)) {
//     return minimize(query);
//   } else {
//     if (pre_cond == query->Pre()->PreCond()) {
//       g_m_list.insert(query->New());
//     }
//   }
// }

// TODO implement callsite inside CFGNode
// if (query->ReachEntry()) {
//   std::string callee = query->GetCallee();
//   std::set<std::string> callers = SnipppetDB::Instance()->QueryCaller(callee);
//   for (std::string caller : callers) {
//     AST *caller_ast = Resource::Instance()->GetAST(caller);
//     if (caller_ast) {
//       std::set<ASTNode*> callsites = caller_ast->GetCallsite(callee);
//       for (ASTNode *callsite : callsites) {
//         Query *q = new Query(query);
//         q->Add(callsite);
//         ret.push_back(q);
//       }
//     }
//   }
//   return ret;
// }


/**
 * TODO random
 * TODO pairwise
 * TODO Symbolic
 * TODO coverage feedback correct path removing
 * TODO mutation test
 */
InputSpec generate_input(std::vector<Variable> invs) {
}

/**
 * TODO m_list
 * TODO coverage feedback remove correct path
 */
// void minimize(Query *query) {
// }
