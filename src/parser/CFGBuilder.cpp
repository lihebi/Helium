#include "helium/parser/Visitor.h"

#include "helium/parser/AST.h"
#include "helium/parser/SourceManager.h"
#include "helium/utils/StringUtils.h"
#include <iostream>

#include "helium/utils/Dot.h"

using std::vector;
using std::string;
using std::map;
using std::set;



void CFGBuilder::pre(ASTNodeBase *node) {
  // node->dump(std::cout);
  // std::cout << std::flush;
}

void CFGBuilder::visit(TokenNode *node) {
  pre(node);
  Visitor::visit(node);
}
void CFGBuilder::visit(TranslationUnitDecl *node) {
  pre(node);
  Visitor::visit(node);
  // Doing nother. We don't want to generate CFG for translation unit
  // CFG *cfg = new CFG();
  // for (ASTNodeBase *n : node->getDecls()) {
  //   CFG *inner = getInnerCFG(n);
  //   cfg->graph.merge(inner->graph);
  //   cfg->ins.insert(inner->ins.begin(), inner->ins.end());
  //   cfg->outs.insert(inner->outs.begin(), inner->outs.end());
  // }
  // addInnerCFG(node, cfg);
}
void CFGBuilder::visit(FunctionDecl *node) {
  pre(node);
  // create a seperate CFG
  // create cfg from function
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *func_node = new CFGNode(node);
  CFGNode *func_out = new CFGNode("func-out");
  cfg->addNode(func_node);
  cfg->addFunc(func_node, node->getName());
  cfg->addNode(func_out);
  CFG *inner = getInnerCFG(node->getBody());
  if (!inner) {
    cfg->graph.addEdge(func_node, func_out);
  } else {
    cfg->graph.merge(inner->graph);
    cfg->graph.addEdge(func_node, inner->ins);
    cfg->graph.addEdge(inner->outs, func_out);

    // handle return: just remove out node
    for (CFGNode *node : return_nodes) {
      // cfg->removeOutEdge(node);
      cfg->addEdge(node, func_out);
    }
  }
  cfg->ins.insert(func_node);
  cfg->outs.insert(func_out);
  addInnerCFG(node, cfg);
  addFuncCFG(node->getName(), cfg);
}


void CFGBuilder::visit(CompoundStmt *node) {
  pre(node);
  Visitor::visit(node);
  // CFGNode *comp_node = new CFGNode(node);
  // cfg->addNode(comp_node);
  // cfg->outs.insert(comp_node);
  // cfg->ins.insert(comp_node);

  std::vector<Stmt*> stmts = node->getBody();
  if (stmts.size() == 0) return;
  CFG *first_inner = nullptr;
  CFG *cfg = new CFG();
  for (Stmt *stmt : stmts) {
    CFG *inner = getInnerCFG(stmt);
    if (!inner) continue;
    if (!first_inner) first_inner = inner;
    cfg->graph.merge(inner->graph);
    cfg->graph.addEdge(cfg->outs, inner->ins);
    cfg->outs = inner->outs;
  }
  if (!first_inner) return;
  cfg->ins = first_inner->ins;
  addInnerCFG(node, cfg);
}

void CFGBuilder::visit(IfStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *if_node = new CFGNode(node);
  CFGNode *if_out = new CFGNode("if-out");
  // callee
  Expr *cond = node->getCond();
  if_node->addCallee(cond->getCallees());
  // add nodes
  cfg->addNode(if_node);
  cfg->addNode(if_out);
  // then (must have)
  CFG *thenCFG = getInnerCFG(node->getThen());
  if (thenCFG) {
    cfg->graph.merge(thenCFG->graph);
    cfg->graph.addEdge(if_node, thenCFG->ins, "true");
    cfg->graph.addEdge(thenCFG->outs, if_out);
  } else {
    cfg->graph.addEdge(if_node, if_out, "true");
  }

  // else
  if (node->getElse()) {
    CFG *elseCFG = getInnerCFG(node->getElse());
    if (elseCFG) {
      cfg->graph.merge(elseCFG->graph);
      cfg->graph.addEdge(if_node, elseCFG->ins, "false");
      cfg->graph.addEdge(elseCFG->outs, if_out);
    } else {
      cfg->graph.addEdge(if_node, if_out, "false");
    }
  } else {
    cfg->graph.addEdge(if_node, if_out, "false");
  }

  cfg->addIn(if_node);
  cfg->addOut(if_out);
  
  addInnerCFG(node, cfg);
}

void CFGBuilder::visit(SwitchStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *switch_node = new CFGNode(node);
  CFGNode *switch_out = new CFGNode("switch-out");
  switch_node->addCallee(node->getCond()->getCallees());
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
    if (inner) {
      cfg->graph.merge(inner->graph);
      cfg->graph.addEdge(switch_node, inner->ins, case_label);
      cfg->graph.addEdge(cfg->outs, inner->ins, "flow-over");
      cfg->outs = inner->outs;
    } else {
      continue;
    }
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
void CFGBuilder::visit(CaseStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *case_node = new CFGNode(node);
  cfg->addNode(case_node);
  cfg->addOut(case_node);

  vector<Stmt*> stmts = node->getBody();
  for (Stmt *stmt : stmts) {
    CFG *inner = getInnerCFG(stmt);
    if (inner) {
      cfg->graph.merge(inner->graph);
      cfg->graph.addEdge(cfg->outs, inner->ins);
      cfg->outs = inner->outs;
    } else {
      continue;
    }
  }

  cfg->addIn(case_node);
  addInnerCFG(node, cfg); 
}
void CFGBuilder::visit(DefaultStmt *node) {
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
    if (inner) {
      cfg->graph.merge(inner->graph);
      cfg->graph.addEdge(cfg->outs, inner->ins);
      cfg->outs = inner->outs;
    } else {
      continue;
    }
  }

  cfg->addIn(def_node);
  addInnerCFG(node, cfg);
}
void CFGBuilder::visit(ForStmt *node) {
  pre(node);
  Visitor::visit(node);

  CFG *cfg = new CFG();
  CFGNode *loop_node = new CFGNode(node);
  CFGNode *loop_out = new CFGNode("loop-out");
  if (node->getInit()) loop_node->addCallee(node->getInit()->getCallees());
  if (node->getCond()) loop_node->addCallee(node->getCond()->getCallees());
  if (node->getInc()) loop_node->addCallee(node->getInc()->getCallees());
  cfg->addNode(loop_node);
  cfg->addNode(loop_out);

  Stmt *body = node->getBody();
  CFG *inner = getInnerCFG(body);
  if (inner) {
    cfg->graph.merge(inner->graph);
    cfg->graph.addEdge(loop_node, inner->ins, "loop-true");
    cfg->graph.addEdge(inner->outs, loop_node, "back");
  } else {
    cfg->graph.addEdge(loop_node, loop_node, "back");
  }

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
void CFGBuilder::visit(WhileStmt *node) {
  pre(node);
  Visitor::visit(node);
  
  CFG *cfg = new CFG();
  CFGNode *loop_node = new CFGNode(node);
  CFGNode *loop_out = new CFGNode("loop-out");
  loop_node->addCallee(node->getCond()->getCallees());
  cfg->addNode(loop_node);
  cfg->addNode(loop_out);

  Stmt *body = node->getBody();
  CFG *inner = getInnerCFG(body);
  if (inner) {
    cfg->graph.merge(inner->graph);
    cfg->graph.addEdge(loop_node, inner->ins, "loop-true");
    cfg->graph.addEdge(inner->outs, loop_node, "back");
  } else {
    cfg->graph.addEdge(loop_node, loop_node, "back");
  }

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
void CFGBuilder::visit(DoStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *loop_node = new CFGNode(node);
  CFGNode *loop_out = new CFGNode("loop-out");
  loop_node->addCallee(node->getCond()->getCallees());
  cfg->addNode(loop_node);
  cfg->addNode(loop_out);

  Stmt *body = node->getBody();
  CFG *inner = getInnerCFG(body);
  if (inner) {
    cfg->graph.merge(inner->graph);
    cfg->graph.addEdge(loop_node, inner->ins, "loop-true");
    cfg->graph.addEdge(inner->outs, loop_node, "back");
  } else {
    cfg->graph.addEdge(loop_node, loop_node, "back");
  }

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

void CFGBuilder::visit(BreakStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *break_node = new CFGNode(node);
  cfg->addNode(break_node);
  cfg->addIn(break_node);
  addInnerCFG(node, cfg);
  break_nodes.insert(break_node);
}
void CFGBuilder::visit(ContinueStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *continue_node = new CFGNode(node);
  cfg->addNode(continue_node);
  cfg->addIn(continue_node);
  addInnerCFG(node, cfg);
  continue_nodes.insert(continue_node);
}
void CFGBuilder::visit(ReturnStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *return_node = new CFGNode(node);
  if (node->getValue()) return_node->addCallee(node->getValue()->getCallees());
  cfg->addNode(return_node);
  cfg->addIn(return_node);
  // cfg->addOut(return_node);
  addInnerCFG(node, cfg);
  return_nodes.insert(return_node);
}
void CFGBuilder::visit(Expr *node) {
  pre(node);
}
void CFGBuilder::visit(DeclStmt *node) {
  pre(node);
  Visitor::visit(node);
  std::string text = node->getText();
  // remove decl if is does not define the variable
  if (m_options.count(CFG_NoDecl) == 1) {
    if (text.find('=') == std::string::npos) {
      return;
    }
  }
  CFG *cfg = new CFG();
  CFGNode *cfgnode = new CFGNode(node);
  cfgnode->addCallee(node->getCallees());
  cfg->addNode(cfgnode);
  cfg->addIn(cfgnode);
  cfg->addOut(cfgnode);
  addInnerCFG(node, cfg);
}
void CFGBuilder::visit(ExprStmt *node) {
  pre(node);
  Visitor::visit(node);
  CFG *cfg = new CFG();
  CFGNode *cfgnode = new CFGNode(node);
  cfgnode->addCallee(node->getCallees());
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


std::string CFG::getDotString() {
  return graph.getDotString
    ([](CFGNode *node)->std::string
     {return node->getLabel();});
}

std::string CFG::getGgxString() {
  return graph.getGgxString
    ([](CFGNode *node)->std::string
     {return node->getLabel();});
}


CFG *create_icfg(std::vector<CFG*> cfgs) {
  // get all function nodes, build a function map
  std::map<std::string, CFGNode*> name2cfgnode;
  CFG *ret = new CFG();
  for (CFG *cfg : cfgs) {
    ret->graph.merge(cfg->graph);
    for (auto &m :cfg->getFuncs()) {
      CFGNode *node = m.first;
      std::string name = m.second;
      name2cfgnode[name] = node;
      // std::cout << name << "\n";
    }
  }
  // std::cout << name2cfgnode.size() << "\n";
  // for all nodes, connect its callees
  for (CFG *cfg : cfgs) {
    for (CFGNode *node : cfg->getAllNodes()) {
      for (std::string callee : node->getCallees()) {
        if (name2cfgnode.count(callee) == 1) {
          // std::cout << "Adding edge" << "\n";
          ret->graph.addEdge(node, name2cfgnode[callee], "Call");
        }
      }
    }
  }
  return ret;
}
