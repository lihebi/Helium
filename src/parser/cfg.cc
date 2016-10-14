#include "cfg.h"
#include "ast_node.h"

#include "utils/dot.h"
#include "utils/utils.h"
#include "resolver/snippet_db.h"
#include "resource.h"

#include "utils/log.h"

#include <iostream>


CFG::CFG() {}
CFG::~CFG() {}

/**
 * Connect this->outs to this node.
 */
void CFG::AddNode(CFGNode *node) {
  helium_print_trace("CFG::AddNode");
  if (!node) return;
  m_nodes.insert(node);
  for (CFGNode *out : m_outs) {
    AddEdge(new CFGEdge(out, node));
  }
  m_outs.clear();
  m_outs.insert(node);
}

void CFG::Merge(CFG *cfg) {
  helium_print_trace("CFG::Merge");
  if (!cfg) return;
  for (CFGNode *node : cfg->GetNodes()) {
    m_nodes.insert(node);
  }
  if (m_outs.empty()) {
    for (CFGNode *cfg_in : cfg->GetIns()) {
      m_ins.insert(cfg_in);
    }
  }
  // copy edges
  copyEdge(cfg);
  // connect this.out to cfg->in, then clear this->out
  for (CFGNode *in : m_outs) {
    for (CFGNode *cfg_in : cfg->GetIns()) {
      AddEdge(new CFGEdge(in, cfg_in));
    }
  }
  m_outs.clear();
  // add cfg->out to this->out
  m_outs = cfg->GetOuts();
}

void CFG::copyEdge(CFG *cfg) {
  helium_print_trace("CFG::copyEdge");
  for (CFGEdge *edge : cfg->Edges()) {
    AddEdge(edge);
  }
  // try to copy m_breaks, m_continues, m_returns here
  for (CFGNode *node : cfg->m_breaks) {
    m_breaks.insert(node);
  }
  for (CFGNode *node : cfg->m_continues) {
    m_continues.insert(node);
  }
  for (CFGNode *node : cfg->m_returns) {
    m_returns.insert(node);
  }
}

void CFG::AdjustReturn() {
  helium_print_trace("CFG::AdjustReturn");
  for (CFGNode *node : m_returns) {
    // assert(m_ins.size() == 1);
    // CFGNode *out = *m_outs.begin();
    // CreateEdge(node, out);
    m_outs.insert(node);
  }
  m_returns.clear();
}

void CFG::AdjustBreak() {
  helium_print_trace("CFG::AdjustBreak");
  for (CFGNode *node : m_breaks) {
    // assert(m_outs.size() == 1);
    // CFGNode *out = *m_outs.begin();
    // CreateEdge(node, out);
    m_outs.insert(node);
  }
  m_breaks.clear();
}

void CFG::AdjustContinue() {
  helium_print_trace("CFG::AdjustContinue");
  // redirect continue node to m_in
  for (CFGNode *node : m_continues) {
    assert(m_ins.size() == 1);
    CFGNode *in = *m_ins.begin();
    AddEdge(new CFGEdge(node, in));
  }
  m_continues.clear();
}

void CFG::MergeBranch(CFG *cfg, bool b) {
  helium_print_trace("CFG::MergeBranch");
  for (CFGNode *node : cfg->GetNodes()) {
    m_nodes.insert(node);
  }
  copyEdge(cfg);
  // add cfg->out to this->out
  for (CFGNode *out : cfg->GetOuts()) {
    m_outs.insert(out);
  }
  // connect this->cond to cfg->in
  if (m_cond) {
    for (CFGNode *in : cfg->GetIns()) {
      AddEdge(new CFGEdge(m_cond, in, (b?"true":"false")));
    }
  }
  m_branch_num++;
}

// TODO case condition as label
void CFG::MergeCase(CFG *cfg, Case *c) {
  helium_print_trace("CFG::MergeCase");
  for (CFGNode *node : cfg->GetNodes()) {
    m_nodes.insert(node);
  }
  // this cond is actually the switch node
  // this is not a good name
  assert(m_cond);
  std::set<CFGNode*> ins = cfg->GetIns();
  if (ins.empty()) {
    m_pending_cases.push_back(c);
  } else {
    assert(ins.size()==1);
    CFGNode *in = *ins.begin();

    // This edge should come with the Case* class
    CFGEdge *edge = new CFGEdge(m_cond, in);
    for (ASTNode *cc : m_pending_cases) {
      edge->AddCase(cc);
    }
    m_pending_cases.clear();
    edge->AddCase(c);
    AddEdge(edge);
    // CreateEdge(m_cond, in, "case: TODO");
    
    for (CFGNode *out : m_last_case_outs) {
      AddEdge(new CFGEdge(out, in));
    }
    m_last_case_outs.clear();
    for (CFGNode *out : cfg->GetOuts()) {
      m_last_case_outs.insert(out);
    }
    copyEdge(cfg);
  }
}

void CFG::MergeDefault(CFG *cfg, Default *def) {
  if (!cfg) {
    // connect last case to the out
    for (CFGNode *out : m_last_case_outs) {
      m_outs.insert(out);
    }
  } else {
    for (CFGNode *node : cfg->GetNodes()) {
      m_nodes.insert(node);
    }
    // connect switch and default
    std::set<CFGNode*> ins = cfg->GetIns();
    assert(ins.size()==1);
    CFGNode *in = *ins.begin();

    
    CFGEdge *edge = new CFGEdge(m_cond, in);
    for (ASTNode *cc : m_pending_cases) {
      edge->AddCase(cc);
    }
    edge->AddCase(def);
    m_pending_cases.clear();
    AddEdge(edge);

    
    // CreateEdge(m_cond, in, "default");
    // connect last case to default in
    for (CFGNode *out : m_last_case_outs) {
      AddEdge(new CFGEdge(out, in));
    }
    // then connect out of default to outs
    for (CFGNode *out : cfg->GetOuts()) {
      m_outs.insert(out);
    }
    copyEdge(cfg);
  }
}

// void CFG::CreateEdge(CFGNode *from, CFGNode *to, std::string label) {
//   helium_print_trace("CFG::CreateEdge");
//   if (!from || !to) return;
//   m_edges[from].insert(to);
//   if (!label.empty()) {
//     m_labels[std::make_pair(from, to)] = label;
//   }
//   // also keep the back edge
//   m_back_edges[to].insert(from);
//   helium_print_trace("CFG::CreateEdge end");
// }


void CFG::Visualize(std::set<CFGNode*> nodesA, std::set<CFGNode*> nodesB, bool open) {
  DotGraph dot;
  // Add node for all the nodes
  for (CFGNode *node : m_nodes) {
    dot.AddNode(node->GetID(), node->GetLabel());
  }
  // color selected nodes
  for (CFGNode *a : nodesA) {
    dot.ColorNode(a->GetID(), DNCK_Yellow);
  }
  for (CFGNode *b : nodesB) {
    dot.ColorNode(b->GetID(), DNCK_Cyan);
  }
  // Add edge for the nodes
  for (CFGEdge *edge : m_edges) {
    dot.AddEdge(edge->From()->GetID(), edge->To()->GetID(), edge->Label());
  }
  // for (auto m : m_edges) {
  //   CFGNode *from = m.first;
  //   for (CFGNode *to : m.second) {
  //     std::string label;
  //     if (m_labels.count({from, to}) == 1) {
  //       label = m_labels[{from, to}];
  //     }
  //     dot.AddEdge(from->GetID(), to->GetID(), label);
  //   }
  // }
  // Add mark for "in" and "out"
  for (CFGNode *in : m_ins) {
    dot.AddText(in->GetID(), "IN");
  }
  // for (CFGNode *out : m_outs) {
  //   dot.AddText(out->GetID(), "OUT");
  // }

  std::string helium_out = "HELIUM_OUT";
  dot.AddNode(helium_out, helium_out);
  dot.AddText(helium_out, helium_out);
  for (CFGNode *out : m_outs) {
    dot.AddEdge(out->GetID(), helium_out);
  }
  std::string dotcode = dot.dump();
  std::string path = utils::visualize_dot_graph(dotcode, open);
  std::cout << "written to " << path  << "\n";
}




/********************************
 * CFG Factory
 *******************************/



/**
 * How to add special edges for break, continue, and return
 *
 * 1. when adding the node, put into m_breaks, and m_returns
 * 2. When for special constructs, consume the m_breaks and m_returns (remove them) and add edges
 *    - switch, while, for, do: break, continue
 *    - when creating with AST, consume return
 * 3. When merging, merge all m_breaks and m_returns
 */



CFG *CFGFactory::CreateCFG(AST *ast) {
  helium_print_trace("CFGFactory::CreateCFG");
  if (!ast) return NULL;
  ASTNode *root = ast->GetRoot();
  CFG *cfg = CreateCFG(root);
  return cfg;
}


CFG *CFGFactory::CreateCFG(ASTNode *node) {
  helium_print_trace("CFGFactory::CreateCFG");
  if (!node) return NULL;
  switch (node->Kind()) {
  case ANK_Stmt: {
    CFG *cfg = new CFG();
    CFGNode *cfgnode = new CFGNode(cfg, node);
    cfg->AddNode(cfgnode);
    cfg->AddIn(cfgnode);
    cfg->AddOut(cfgnode);


    // If it is break, continue, AddBreakNode
    XMLNodeKind kind = xmlnode_to_kind(node->GetXMLNode());
    if (kind == NK_Break) {
      cfg->AddBreak(cfgnode);
      cfg->RemoveOut(cfgnode);
    } else if (kind == NK_Continue) {
      cfg->AddContinue(cfgnode);
      cfg->RemoveOut(cfgnode);
    } else if (kind == NK_Return) {
      cfg->AddReturn(cfgnode);
      cfg->RemoveOut(cfgnode);
    }
    // If it is return, AddReturnNode

    
    return cfg;
  }
  case ANK_If: return CreateCFGFromIf(dynamic_cast<If*>(node)); break;
  case ANK_Function: return CreateCFGFromFunction(dynamic_cast<Function*>(node)); break;
  case ANK_ElseIf: return CreateCFGFromElseIf(dynamic_cast<ElseIf*>(node)); break;
    // the following 4 types need to adjust breaks nodes
  case ANK_Switch: return CreateCFGFromSwitch(dynamic_cast<Switch*>(node)); break;
  case ANK_While: return CreateCFGFromWhile(dynamic_cast<While*>(node)); break;
  case ANK_For: return CreateCFGFromFor(dynamic_cast<For*>(node)); break;
  case ANK_Do: return CreateCFGFromDo(dynamic_cast<Do*>(node)); break;

    
  case ANK_Block: return CreateCFGFromBlock(dynamic_cast<Block*>(node)); break;
  default: return NULL;
  }
}
CFG *CFGFactory::CreateCFGFromIf(If *astnode) {
  helium_print_trace("CFGFactory::CreateCFGFromIf");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(cfg, astnode);
  cfg->AddNode(node);
  cfg->AddIn(node);
  cfg->AddOut(node);
  cfg->SetCond(node);
  
  Then *then_node = astnode->GetThen();
  Else *else_node = astnode->GetElse();
  std::vector<ElseIf*> elseifs = astnode->GetElseIfs();
  // then
  if (then_node) {
    CFG *branch_cfg = new CFG();
    for (ASTNode *child : then_node->Children()) {
      CFG *child_cfg = CreateCFG(child);
      branch_cfg->Merge(child_cfg);
    }
    cfg->MergeBranch(branch_cfg, true);
  }
  // else if
  CFG *current;
  current = cfg;
  for (ElseIf* else_if : elseifs) {
    CFG *elseif_cfg = CreateCFGFromElseIf(else_if);
    current->MergeBranch(elseif_cfg, false);
    current = elseif_cfg;
  }
  // else
  if (else_node) {
    CFG *branch_cfg = new CFG();
    for (ASTNode *child : else_node->Children()) {
      CFG *child_cfg = CreateCFG(child);
      branch_cfg->Merge(child_cfg);
    }
    current->MergeBranch(branch_cfg, false);
  }

  if (cfg->GetBranchNum() == 2) {
    cfg->RemoveOut(node);
  }
  return cfg;
}
CFG *CFGFactory::CreateCFGFromFunction(Function *astnode) {
  helium_print_trace("CFGFactory::CreateCFGFromFunction");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(cfg, astnode);
  cfg->AddNode(node);
  cfg->AddIn(node);
  for (ASTNode *child : astnode->Children()) {
    CFG *child_cfg = CreateCFG(child);
    if (child_cfg) {
      cfg->Merge(child_cfg);
    }
  }
  // this is from the root of AST, take care of AdjustReturn
  cfg->AdjustReturn();
  return cfg;
}
CFG *CFGFactory::CreateCFGFromElseIf(ElseIf *astnode) {
  helium_print_trace("CFGFactory::CreateCFGFromElseIf");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(cfg, astnode);
  cfg->AddNode(node);
  cfg->AddIn(node);
  CFG *branch_cfg = new CFG();
  for (ASTNode *child : astnode->Children()) {
    CFG *child_cfg = CreateCFG(child);
    branch_cfg->Merge(child_cfg);
  }
  cfg->SetCond(node);
  cfg->MergeBranch(branch_cfg, true);
  return cfg;
}




/**
 * switch
 *
 * (HEBI: Switch Control Flow)
 * Connect all the case clauses, tail to head
 * Then adjust break and continue.
 * Record the condition on the edge.
 * If there may be multiple conditions for a case clause.
 */
CFG *CFGFactory::CreateCFGFromSwitch(Switch *astnode) {
  helium_print_trace("CFGFactory::CreateCFGFromSwitch");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(cfg, astnode);
  cfg->AddNode(node);
  cfg->AddIn(node);
  cfg->SetCond(node);
  cfg->RemoveOut(node); // switch itself should not be in outs
  std::vector<Case*> cases = astnode->GetCases();
  std::vector<CFG*> case_cfgs;
  for (Case *c : cases) {
    CFG *case_cfg = new CFG();
    for (ASTNode *child : c->Children()) {
      CFG *child_cfg = CreateCFG(child);
      case_cfg->Merge(child_cfg);
    }
    case_cfgs.push_back(case_cfg);
    cfg->MergeCase(case_cfg, c);
  }
  Default *def = astnode->GetDefault();
  if (!def) {
    cfg->MergeDefault(NULL, NULL);
  } else {
    CFG *def_cfg = new CFG();
    for (ASTNode *child : def->Children()) {
      CFG *child_cfg = CreateCFG(child);
      def_cfg->Merge(child_cfg);
    }
    cfg->MergeDefault(def_cfg, def);
  }
  cfg->AdjustBreak();
  return cfg;
}

// while
CFG *CFGFactory::CreateCFGFromWhile(While *astnode) {
  helium_print_trace("CFGFactory::CreateCFGFromWhile");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(cfg, astnode);
  cfg->AddNode(node);
  cfg->AddIn(node);
  cfg->AddOut(node);
  CFG *body_cfg = new CFG();
  for (ASTNode *child : astnode->Children()) {
    CFG *child_cfg = CreateCFG(child);
    body_cfg->Merge(child_cfg);
  }

  cfg->Merge(body_cfg);
  for (CFGNode *out : body_cfg->GetOuts()) {
    // cfg->CreateEdge(out, node, "B");
    cfg->AddEdge(new CFGEdge(out, node, "B"));
    cfg->RemoveOut(out);
  }
  cfg->AddOut(node);

  cfg->AdjustBreak();
  cfg->AdjustContinue();
  return cfg;
}
/**
 * Exactly the same as While
 */
CFG *CFGFactory::CreateCFGFromFor(For *astnode) {
  helium_print_trace("CFGFactory::CreateCFGFromFor");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(cfg, astnode);
  cfg->AddNode(node);
  cfg->AddIn(node);
  cfg->AddOut(node);
  CFG *body_cfg = new CFG();
  for (ASTNode *child : astnode->Children()) {
    CFG *child_cfg = CreateCFG(child);
    body_cfg->Merge(child_cfg);
  }


  // std::cout << "cfg->Out: " << cfg->GetOuts().size()  << "\n";
  // std::cout << "body->In: " << body_cfg->GetIns().size()  << "\n";
  
  cfg->Merge(body_cfg);
  for (CFGNode *out : body_cfg->GetOuts()) {
    cfg->AddEdge(new CFGEdge(out, node, "B"));
    // cfg->CreateEdge(out, node, "B");
    cfg->RemoveOut(out);
  }
  cfg->AddOut(node);
  
  // std::cout << "For: "  << "\n";
  // std::cout << "cfg->Out: " << cfg->GetOuts().size()  << "\n";
  // std::cout << "cfg->In: " << cfg->GetIns().size()  << "\n";
  
  // cfg->Visualize();


  cfg->AdjustBreak();
  cfg->AdjustContinue();
  return cfg;
}
CFG *CFGFactory::CreateCFGFromDo(Do *astnode) {
  helium_print_trace("CFGFactory::CreateCFGFromDo");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(cfg, astnode);

  CFG *body_cfg = new CFG();
  for (ASTNode *child : astnode->Children()) {
    CFG *child_cfg = CreateCFG(child);
    body_cfg->Merge(child_cfg);
  }
  // body_cfg->Visualize();

  
  cfg->Merge(body_cfg);
  
  cfg->AddNode(node);
  cfg->AddOut(node);

  for (CFGNode *in : body_cfg->GetIns()) {
    cfg->AddEdge(new CFGEdge(node, in, "B"));
  }

  cfg->AdjustBreak();
  cfg->AdjustContinue();
  return cfg;
}


CFG *CFGFactory::CreateCFGFromBlock(Block *astnode) {
  helium_print_trace("CFGFactory::CreateCFGFromBlock");
  CFG *cfg = new CFG();
  CFG *body_cfg = new CFG();
  for (ASTNode *child : astnode->Children()) {
    CFG *child_cfg = CreateCFG(child);
    body_cfg->Merge(child_cfg);
  }
  cfg->Merge(body_cfg);
  return cfg;
}



std::set<CFGNode*> CFG::GetPredecessors(CFGNode *node) {
  std::set<CFGNode*> ret;
  if (m_edge_back_idx.count(node)==1) {
    for (CFGEdge *edge : m_edge_back_idx[node]) {
      ret.insert(edge->From());
    }
  }
  return ret;
}

std::set<CFGNode*> CFG::GetInterPredecessors(CFGNode *node) {
  std::set<CFGNode*> ret;
  if (m_edge_back_idx.count(node) == 1) {
    return ret;
  }
  // ICFG
  ASTNode *astnode = node->GetASTNode();
  // assert(astnode->Kind() == ANK_Function);
  if (astnode->Kind() != ANK_Function) {
    std::cerr << "EE: the CFG node is not a function, but has no predecessor." << "\n";
    return ret;
  }
  std::string func = astnode->GetAST()->GetFunctionName();
  std::cout << "Getting interprocedure predecessor from ICFG, function: " << func  << "\n";


  AST *ast = astnode->GetAST();
  int id = Resource::Instance()->GetASTID(ast);
  if (id == -1) {
    std::cerr << "Cannot find AST for func " << func << "\n";
    exit(1);
  }

  std::set<int> caller_ids = SnippetDB::Instance()->QueryCallers(id);
  for (int caller_id : caller_ids) {
    AST *ast = Resource::Instance()->GetAST(caller_id);
    CFG *cfg = Resource::Instance()->GetCFG(caller_id);
    if (!ast) continue;
    std::set<ASTNode*> callsites = ast->GetCallSites(func);
    for (ASTNode * callsite : callsites) {
      CFGNode *cfgnode = cfg->ASTNodeToCFGNode(callsite);
      ret.insert(cfgnode);
    }
  }
  std::cout << "Predecessor count: " << ret.size() << "\n";
  // predecessor line
  for (CFGNode *node : ret) {
    std::cout << node->GetLabel() << "\n";
  }
  return ret;
}


