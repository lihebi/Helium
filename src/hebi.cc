#include "hebi.h"
#include "parser/xml_doc_reader.h"
#include "utils/log.h"
#include "workflow/resource.h"

#include "common.h"
#include "config/options.h"

#include "workflow/builder.h"
#include "code_gen.h"

#include <iostream>


Query::Query(ASTNode *astnode) {
  assert(astnode);
  CFG *cfg = Resource::Instance()->GetCFG(astnode->GetAST());
  CFGNode *cfgnode = cfg->ASTNodeToCFGNode(astnode);
  m_nodes.insert(cfgnode);
  m_new = cfgnode;
}

/**
 * Visualize on CFG.
 * Only display the first CFG for now.
 */
void Query::Visualize(bool open) {
  ASTNode *astnode = m_new->GetASTNode();
  CFG *cfg = Resource::Instance()->GetCFG(astnode->GetAST());
  // these nodes may not belong to this cfg
  cfg->Visualize(m_nodes, {m_new}, open);
}



std::deque<Query*> g_worklist;
std::map<CFGNode*, std::set<Query*> > g_waiting_quries;
std::map<Query*, std::set<Query*> > g_propagating_queries;

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

void Query::ResolveInput() {
  std::cout << "Query::ResolveInput"  << "\n";
  // for all the ASTs, resolve input
  // TODO should we supply input when necessary? Based on the output instrumentation during run time?
  m_inputs.clear();
  CFG *cfg = m_new->GetCFG();
  // ASTNode *astnode = m_new->GetASTNode();
  // AST *ast = astnode->GetAST();
  std::set<ASTNode*> first_astnodes;
  for (CFGNode *cfgnode : m_nodes) {
    if (cfg->Contains(cfgnode)) {
      first_astnodes.insert(cfgnode->GetASTNode());
    }
  }
  // std::map<std::string, Type*> inputs;
  for (ASTNode *astnode : first_astnodes) {
    std::set<std::string> ids = astnode->GetVarIds();
    for (std::string id : ids) {
      if (id.empty()) continue;
      if (is_c_keyword(id)) continue;
      std::cout << id  << "\n";
      SymbolTable *tbl = astnode->GetSymbolTable();
      tbl->dump();
      SymbolTableValue *st_value = tbl->LookUp(id);
      if (st_value) {
        if (first_astnodes.count(st_value->GetNode()) == 0) {
          // input
          m_inputs[st_value->GetName()] = st_value->GetType();
        }
      } else {
        // TODO global
        // Type *type = GlobalVariableRegistry::Instance()->LookUp(id);
      }
    }
  }
  std::cout << "input size: " << m_inputs.size()  << "\n";
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
  generator.Gen();
  m_main = generator.GetMain();
  m_support = generator.GetSupport();
  m_makefile = generator.GetMakefile();
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
  Query *init_query = new Query(node);
  g_worklist.push_back(init_query);
  init_query->Visualize();
  // init(node);
  while (!g_worklist.empty()) {
    // std::cout << "size of worklist: " << g_worklist.size()  << "\n";
    Query *query = g_worklist.front();
    g_worklist.pop_front();
    // query->Visualize();
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
    std::cout << "Wrote to: " << builder.GetDir()  << "\n";
    if (builder.Success()) {
      utils::print("compile success\n", utils::CK_Green);
      std::string executable = builder.GetExecutable();
    } else {
      utils::print("compile error\n", utils::CK_Red);
    }
    // std::string executable = write_and_compile(code);
    
    // TODO
    // InputSpec input = generate_input(invs);
    // Profile profile = test(executable, input);
    // BugSig *bs = oracle(input, profile);
    // if (bs) {
    //   Merge(BS, bs);
    // } else {
      std::vector<Query*> queries = select(query);
      g_worklist.insert(g_worklist.end(), queries.begin(), queries.end());
      std::cout << "---"  << "\n";
      for (Query *q : queries) {
        q->Visualize();
      }
    // }
  }
}


/**
 * TODO context search
 * , Profile profile
 */
std::vector<Query*> select(Query *query) {
  print_trace("select");
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
    Query *q = new Query(*query);
    q->Add(pred);
    ret.push_back(q);
  }

  // update g_propagating_queries
  g_propagating_queries[query].insert(ret.begin(), ret.end());
  return ret;
}


std::set<Query*> find_mergable_query(CFGNode *node, Query *orig_query) {
  print_trace("find_mergable_query");
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
// InputSpec generate_input(std::vector<Variable> invs) {
// }

/**
 * TODO m_list
 * TODO coverage feedback remove correct path
 */
// void minimize(Query *query) {
// }
