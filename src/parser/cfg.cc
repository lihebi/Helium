#include "cfg.h"
#include "ast_node.h"

#include "utils/dot.h"
#include "utils/utils.h"
#include "config/options.h"

#include <iostream>


CFG::CFG() {}
CFG::~CFG() {}

/**
 * Connect this->outs to this node.
 */
void CFG::AddNode(CFGNode *node) {
  print_trace("CFG::AddNode");
  if (!node) return;
  m_nodes.insert(node);
  for (CFGNode *out : m_outs) {
    CreateEdge(out, node);
  }
  m_outs.clear();
  m_outs.insert(node);
}

void CFG::Merge(CFG *cfg) {
  print_trace("CFG::Merge");
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
      CreateEdge(in, cfg_in);
    }
  }
  m_outs.clear();
  // add cfg->out to this->out
  m_outs = cfg->GetOuts();
}

void CFG::copyEdge(CFG *cfg) {
  print_trace("CFG::copyEdge");
  for (auto m : cfg->m_edges) {
    CFGNode *from = m.first;
    for (CFGNode *to : m.second) {
      CreateEdge(from, to);
    }
  }
  for (auto m : cfg->m_labels) {
    m_labels[m.first] = m.second;
  }
}

void CFG::MergeBranch(CFG *cfg, bool b) {
  print_trace("CFG::MergeBranch");
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
      CreateEdge(m_cond, in, (b?"true":"false"));
    }
  }
  m_branch_num++;
}

// TODO case condition as label
void CFG::MergeCase(CFG *cfg) {
  print_trace("CFG::MergeCase");
  for (CFGNode *node : cfg->GetNodes()) {
    m_nodes.insert(node);
  }
  for (CFGNode *out : cfg->GetOuts()) {
    m_outs.insert(out);
  }
  if (m_cond) {
    for (CFGNode *in : cfg->GetIns()) {
      CreateEdge(m_cond, in, "case: TODO");
    }
  }
  copyEdge(cfg);
}

void CFG::CreateEdge(CFGNode *from, CFGNode *to, std::string label) {
  print_trace("CFG::CreateEdge");
  if (!from || !to) return;
  m_edges[from].insert(to);
  // TODO also keep the back edge
  if (!label.empty()) {
    m_labels[std::make_pair(from, to)] = label;
  }
}


void CFG::Visualize() {
  DotGraph dot;
  // Add node for all the nodes
  for (CFGNode *node : m_nodes) {
    dot.AddNode(node->GetID(), node->GetLabel());
  }
  // Add edge for the nodes
  for (auto m : m_edges) {
    CFGNode *from = m.first;
    for (CFGNode *to : m.second) {
      std::string label;
      if (m_labels.count({from, to}) == 1) {
        label = m_labels[{from, to}];
      }
      dot.AddEdge(from->GetID(), to->GetID(), label);
    }
  }
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
  std::string path = utils::visualize_dot_graph(dotcode);
  std::cout << "written to " << path  << "\n";
}


/********************************
 * CFG Factory
 *******************************/






CFG *CFGFactory::CreateCFG(AST *ast) {
  print_trace("CFGFactory::CreateCFG");
  if (!ast) return NULL;
  ASTNode *root = ast->GetRoot();
  return CreateCFG(root);
}


CFG *CFGFactory::CreateCFG(ASTNode *node) {
  print_trace("CFGFactory::CreateCFG");
  if (!node) return NULL;
  switch (node->Kind()) {
  case ANK_Stmt: {
    CFG *cfg = new CFG();
    CFGNode *cfgnode = new CFGNode(node);
    cfg->AddNode(cfgnode);
    cfg->AddIn(cfgnode);
    cfg->AddOut(cfgnode);
    return cfg;
  }
  case ANK_If: return CreateCFGFromIf(dynamic_cast<If*>(node)); break;
  case ANK_Function: return CreateCFGFromFunction(dynamic_cast<Function*>(node)); break;
  case ANK_ElseIf: return CreateCFGFromElseIf(dynamic_cast<ElseIf*>(node)); break;
  case ANK_Switch: return CreateCFGFromSwitch(dynamic_cast<Switch*>(node)); break;
  case ANK_While: return CreateCFGFromWhile(dynamic_cast<While*>(node)); break;
  case ANK_For: return CreateCFGFromFor(dynamic_cast<For*>(node)); break;
  case ANK_Do: return CreateCFGFromDo(dynamic_cast<Do*>(node)); break;
  case ANK_Block: return CreateCFGFromBlock(dynamic_cast<Block*>(node)); break;
  default: return NULL;
  }
}
CFG *CFGFactory::CreateCFGFromIf(If *astnode) {
  print_trace("CFGFactory::CreateCFGFromIf");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(astnode);
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
  print_trace("CFGFactory::CreateCFGFromFunction");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(astnode);
  cfg->AddNode(node);
  cfg->AddIn(node);
  for (ASTNode *child : astnode->Children()) {
    CFG *child_cfg = CreateCFG(child);
    if (child_cfg) {
      cfg->Merge(child_cfg);
    }
  }
  return cfg;
}
CFG *CFGFactory::CreateCFGFromElseIf(ElseIf *astnode) {
  print_trace("CFGFactory::CreateCFGFromElseIf");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(astnode);
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
CFG *CFGFactory::CreateCFGFromSwitch(Switch *astnode) {
  print_trace("CFGFactory::CreateCFGFromSwitch");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(astnode);
  cfg->AddNode(node);
  cfg->AddIn(node);
  cfg->SetCond(node);
  cfg->RemoveOut(node); // switch itself should not be in outs
  std::vector<Case*> cases = astnode->GetCases();
  for (Case *c : cases) {
    CFG *case_cfg = new CFG();
    for (ASTNode *child : c->Children()) {
      CFG *child_cfg = CreateCFG(child);
      case_cfg->Merge(child_cfg);
    }
    cfg->MergeCase(case_cfg);
  }
  // TODO default
  return cfg;
}
CFG *CFGFactory::CreateCFGFromWhile(While *astnode) {
  print_trace("CFGFactory::CreateCFGFromWhile");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(astnode);
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
    cfg->CreateEdge(out, node, "B");
    cfg->RemoveOut(out);
  }
  cfg->AddOut(node);
  return cfg;
}
/**
 * Exactly the same as While
 */
CFG *CFGFactory::CreateCFGFromFor(For *astnode) {
  print_trace("CFGFactory::CreateCFGFromFor");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(astnode);
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
    cfg->CreateEdge(out, node, "B");
    cfg->RemoveOut(out);
  }
  cfg->AddOut(node);
  
  // std::cout << "For: "  << "\n";
  // std::cout << "cfg->Out: " << cfg->GetOuts().size()  << "\n";
  // std::cout << "cfg->In: " << cfg->GetIns().size()  << "\n";
  
  // cfg->Visualize();
  return cfg;
}
CFG *CFGFactory::CreateCFGFromDo(Do *astnode) {
  print_trace("CFGFactory::CreateCFGFromDo");
  CFG *cfg = new CFG();
  CFGNode *node = new CFGNode(astnode);

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
    cfg->CreateEdge(node, in, "B");
  }
  return cfg;
}


CFG *CFGFactory::CreateCFGFromBlock(Block *astnode) {
  print_trace("CFGFactory::CreateCFGFromBlock");
  CFG *cfg = new CFG();
  CFG *body_cfg = new CFG();
  for (ASTNode *child : astnode->Children()) {
    CFG *child_cfg = CreateCFG(child);
    body_cfg->Merge(child_cfg);
  }
  cfg->Merge(body_cfg);
  return cfg;
}









