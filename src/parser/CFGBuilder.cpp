#include "helium/parser/visitor.h"

#include "helium/parser/ast_v2.h"
#include "helium/parser/source_manager.h"
#include "helium/utils/string_utils.h"
#include <iostream>

#include "helium/utils/dot.h"

using std::vector;
using std::string;
using std::map;
using std::set;

using namespace v2;

void CFGBuilder::pre(v2::ASTNodeBase *node) {
  // node->dump(std::cout);
  // std::cout << std::flush;
}

void CFGBuilder::visit(v2::TokenNode *node) {
  pre(node);
  Visitor::visit(node);
}
void CFGBuilder::visit(v2::TranslationUnitDecl *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  for (ASTNodeBase *n : node->getDecls()) {
    CFG *inner = getInnerCFG(n);
    cfg->graph.merge(inner->graph);
    cfg->ins.insert(inner->ins.begin(), inner->ins.end());
    cfg->outs.insert(inner->outs.begin(), inner->outs.end());
  }
  addInnerCFG(node, cfg);
}
void CFGBuilder::visit(v2::FunctionDecl *node) {
  pre(node);
  // create a seperate CFG
  // create cfg from function
  Visitor::visit(node);
  v2::CFG *cfg = new CFG();
  v2::CFGNode *func_node = new CFGNode(node);
  v2::CFGNode *func_out = new CFGNode("func-out");
  cfg->addNode(func_node);
  cfg->addNode(func_out);
  v2::CFG *inner = getInnerCFG(node->getBody());
  cfg->graph.merge(inner->graph);
  cfg->graph.addEdge(func_node, inner->ins);
  cfg->graph.addEdge(inner->outs, func_out);

  // handle return: just remove out node
  for (CFGNode *node : return_nodes) {
    // cfg->removeOutEdge(node);
    cfg->addEdge(node, func_out);
  }
  
  addInnerCFG(node, cfg);
}
void CFGBuilder::visit(v2::CompoundStmt *node) {
  pre(node);
  Visitor::visit(node);
  v2::CFG *cfg = new CFG();
  v2::CFGNode *comp_node = new CFGNode(node);
  cfg->addNode(comp_node);
  cfg->outs.insert(comp_node);
  cfg->ins.insert(comp_node);

  std::vector<Stmt*> stmts = node->getBody();
  for (Stmt *stmt : stmts) {
    CFG *inner = getInnerCFG(stmt);
    cfg->graph.merge(inner->graph);
    cfg->graph.addEdge(cfg->outs, inner->ins);
    cfg->outs = inner->outs;
  }
  
  addInnerCFG(node, cfg);
}

void CFGBuilder::visit(v2::IfStmt *node) {
  pre(node);
  Visitor::visit(node);
  v2::CFG *cfg = new CFG();
  v2::CFGNode *if_node = new CFGNode(node);
  v2::CFGNode *if_out = new CFGNode("if-out");
  cfg->addNode(if_node);
  cfg->addNode(if_out);
  // then (must have)
  CFG *thenCFG = getInnerCFG(node->getThen());
  cfg->graph.merge(thenCFG->graph);
  cfg->graph.addEdge(if_node, thenCFG->ins, "true");
  cfg->graph.addEdge(thenCFG->outs, if_out);

  // else
  if (node->getElse()) {
    CFG *elseCFG = getInnerCFG(node->getElse());
    cfg->graph.merge(elseCFG->graph);
    cfg->graph.addEdge(if_node, elseCFG->ins, "false");
    cfg->graph.addEdge(elseCFG->outs, if_out);
  } else {
    cfg->graph.addEdge(if_node, if_out, "false");
  }

  cfg->addIn(if_node);
  cfg->addOut(if_out);
  
  addInnerCFG(node, cfg);
}

void CFGBuilder::visit(v2::SwitchStmt *node) {
  pre(node);
  Visitor::visit(node);
  v2::CFG *cfg = new CFG();
  v2::CFGNode *switch_node = new CFGNode(node);
  CFGNode *switch_out = new CFGNode("switch-out");
  cfg->addNode(switch_node);
  cfg->addNode(switch_out);
  
  std::vector<SwitchCase*> cases = node->getCases();
  for (SwitchCase *c : cases) {
    // get case condition
    std::string case_label;
    if (dynamic_cast<CaseStmt*>(c)) {
      case_label = dynamic_cast<CaseStmt*>(c)->getCond()->getText();
    } else if (dynamic_cast<DefaultStmt*>(c)) {
      case_label = "default";
    }
    CFG *inner = getInnerCFG(c);
    cfg->graph.merge(inner->graph);
    cfg->graph.addEdge(switch_node, inner->ins, case_label);
    cfg->graph.addEdge(cfg->outs, inner->ins, "flow-over");
    cfg->outs = inner->outs;
  }

  cfg->graph.addEdge(cfg->outs, switch_out);
  // clear the last case/default out
  cfg->outs.clear();

  // handle break
  for (CFGNode *node : break_nodes) {
    // cfg->removeOutEdge(node);
    cfg->addEdge(node, switch_out);
  }
  break_nodes.clear();

  cfg->ins.insert(switch_node);
  cfg->outs.insert(switch_out);
  
  addInnerCFG(node, cfg);
}
void CFGBuilder::visit(v2::CaseStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *case_node = new CFGNode(node);
  cfg->addNode(case_node);
  cfg->addOut(case_node);

  vector<Stmt*> stmts = node->getBody();
  for (Stmt *stmt : stmts) {
    CFG *inner = getInnerCFG(stmt);
    cfg->graph.merge(inner->graph);
    cfg->graph.addEdge(cfg->outs, inner->ins);
    cfg->outs = inner->outs;
  }

  cfg->addIn(case_node);
  addInnerCFG(node, cfg); 
}
void CFGBuilder::visit(v2::DefaultStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *def_node = new CFGNode(node);
  cfg->addNode(def_node);
  cfg->addOut(def_node);

  vector<Stmt*> stmts = node->getBody();
  std::cout << "Default has: " << stmts.size() << "\n";
  for (Stmt *stmt : stmts) {
    CFG *inner = getInnerCFG(stmt);
    cfg->graph.merge(inner->graph);
    cfg->graph.addEdge(cfg->outs, inner->ins);
    cfg->outs = inner->outs;
  }

  cfg->addIn(def_node);
  addInnerCFG(node, cfg);
}
void CFGBuilder::visit(v2::ForStmt *node) {
  pre(node);
  Visitor::visit(node);

  CFG *cfg = new v2::CFG();
  CFGNode *loop_node = new v2::CFGNode(node);
  v2::CFGNode *loop_out = new v2::CFGNode("loop-out");
  cfg->addNode(loop_node);
  cfg->addNode(loop_out);

  Stmt *body = node->getBody();
  CFG *inner = getInnerCFG(body);
  cfg->graph.merge(inner->graph);
  cfg->graph.addEdge(loop_node, inner->ins, "loop-true");
  cfg->graph.addEdge(inner->outs, loop_node, "back");

  // cfg->outs.insert(break_nodes.begin(), break_nodes.end());
  cfg->graph.addEdge(break_nodes, loop_out, "break");
  break_nodes.clear();
  cfg->graph.addEdge(continue_nodes, loop_node, "continue");
  continue_nodes.clear();

  cfg->addIn(loop_node);
  cfg->graph.addEdge(loop_node, loop_out, "loop-false");
  cfg->addOut(loop_out);

  addInnerCFG(node, cfg);
}
void CFGBuilder::visit(v2::WhileStmt *node) {
  pre(node);
  Visitor::visit(node);
  
  CFG *cfg = new v2::CFG();
  CFGNode *loop_node = new v2::CFGNode(node);
  v2::CFGNode *loop_out = new v2::CFGNode("loop-out");
  cfg->addNode(loop_node);
  cfg->addNode(loop_out);

  Stmt *body = node->getBody();
  CFG *inner = getInnerCFG(body);
  cfg->graph.merge(inner->graph);
  cfg->graph.addEdge(loop_node, inner->ins, "loop-true");
  cfg->graph.addEdge(inner->outs, loop_node, "back");

  // cfg->outs.insert(break_nodes.begin(), break_nodes.end());
  cfg->graph.addEdge(break_nodes, loop_out, "break");
  break_nodes.clear();
  cfg->graph.addEdge(continue_nodes, loop_node, "continue");
  continue_nodes.clear();

  cfg->addIn(loop_node);
  cfg->graph.addEdge(loop_node, loop_out, "loop-false");
  cfg->addOut(loop_out);

  addInnerCFG(node, cfg);
}
void CFGBuilder::visit(v2::DoStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new v2::CFG();
  CFGNode *loop_node = new v2::CFGNode(node);
  v2::CFGNode *loop_out = new v2::CFGNode("loop-out");
  cfg->addNode(loop_node);
  cfg->addNode(loop_out);

  Stmt *body = node->getBody();
  CFG *inner = getInnerCFG(body);
  cfg->graph.merge(inner->graph);
  cfg->graph.addEdge(loop_node, inner->ins, "loop-true");
  cfg->graph.addEdge(inner->outs, loop_node, "back");

  // cfg->outs.insert(break_nodes.begin(), break_nodes.end());
  cfg->graph.addEdge(break_nodes, loop_out, "break");
  break_nodes.clear();
  cfg->graph.addEdge(continue_nodes, loop_node, "continue");
  continue_nodes.clear();

  cfg->addIn(loop_node);
  cfg->graph.addEdge(loop_node, loop_out, "loop-false");
  cfg->addOut(loop_out);

  addInnerCFG(node, cfg);
}

void CFGBuilder::visit(v2::BreakStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *break_node = new CFGNode(node);
  cfg->addNode(break_node);
  cfg->addIn(break_node);
  addInnerCFG(node, cfg);
  break_nodes.insert(break_node);
}
void CFGBuilder::visit(v2::ContinueStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *continue_node = new CFGNode(node);
  cfg->addNode(continue_node);
  cfg->addIn(continue_node);
  addInnerCFG(node, cfg);
  continue_nodes.insert(continue_node);
}
void CFGBuilder::visit(v2::ReturnStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *return_node = new CFGNode(node);
  cfg->addNode(return_node);
  cfg->addIn(return_node);
  // cfg->addOut(return_node);
  addInnerCFG(node, cfg);
  return_nodes.insert(return_node);
}
void CFGBuilder::visit(v2::Expr *node) {
  pre(node);
}
void CFGBuilder::visit(v2::DeclStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *cfgnode = new CFGNode(node);
  cfg->addNode(cfgnode);
  cfg->addIn(cfgnode);
  cfg->addOut(cfgnode);
  addInnerCFG(node, cfg);
}
void CFGBuilder::visit(v2::ExprStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *cfgnode = new CFGNode(node);
  cfg->addNode(cfgnode);
  cfg->addIn(cfgnode);
  cfg->addOut(cfgnode);
  addInnerCFG(node, cfg);
}


std::string CFGNode::getLabel() {
  if (astnode) {
    std::ostringstream ss;
    astnode->dump(ss);
    return ss.str();
  } else {
    assert(!dummy.empty());
    return dummy;
  }
}


std::string CFG::visualize() {
  // TODO
  return graph.visualize([](CFGNode *node)->std::string{return node->getLabel();});
  // return hebigraph::visualize(graph);
  // DotGraph graph;
  // std::map<CFGNode*, int> IDs;
  // int ID=0;
  // for (CFGNode* node : allNodes) {
  //   assert(node);
  //   graph.AddNode(std::to_string(ID), node->getLabel());
  //   IDs[node]=ID;
  //   ID++;
  // }
  // for (auto m : edges) {
  //   graph.AddEdge(std::to_string(IDs[m.first]),
  //                 std::to_string(IDs[m.second]));
  // }
  // std::string dotstring = graph.dump();
  // return visualize_dot_graph(dotstring);
}

