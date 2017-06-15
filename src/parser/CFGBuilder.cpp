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

void CFGBuilder::createCurrent(v2::ASTNodeBase *node) {
  cur_cfg = new v2::CFG();
  cur_cfgnode = new v2::CFGNode(node);
  cur_cfg->addNode(cur_cfgnode);
  cur_cfg->setIn(cur_cfgnode);
  cur_cfg->setOut(cur_cfgnode);
}

void CFGBuilder::visit(v2::TokenNode *node) {
  pre(node);
  Visitor::visit(node);
}
void CFGBuilder::visit(v2::TranslationUnitDecl *node) {
  pre(node);
  Visitor::visit(node);
}
void CFGBuilder::visit(v2::FunctionDecl *node) {
  pre(node);
  // create a seperate CFG
  // create cfg from function
  Visitor::visit(node);
  v2::CFG *bodyCFG = getInnerCFG(node->getBody());

  createCurrent(node);
  
  cur_cfg->merge(bodyCFG);
  
  addInnerCFG(node, cur_cfg);
}
void CFGBuilder::visit(v2::CompoundStmt *node) {
  pre(node);
  Visitor::visit(node);
  std::vector<Stmt*> stmts = node->getBody();
  createCurrent(node);
  for (Stmt *stmt : stmts) {
    cur_cfg->merge(getInnerCFG(stmt));
  }
  addInnerCFG(node, cur_cfg);
}

void CFGBuilder::visit(v2::IfStmt *node) {
  pre(node);
  Visitor::visit(node);
  createCurrent(node);

  if (node->getThen()) {
    CFG *thenCFG = getInnerCFG(node->getThen());
    cur_cfg->mergeBranch(thenCFG, cur_cfgnode, true);
  }
  if (node->getElse()) {
    CFG *elseCFG = getInnerCFG(node->getElse());
    cur_cfg->mergeBranch(elseCFG, cur_cfgnode, false);
  }
  addInnerCFG(node, cur_cfg);
}

void CFGBuilder::visit(v2::SwitchStmt *node) {
  pre(node);
  Visitor::visit(node);

  createCurrent(node);
  
  vector<SwitchCase*> cases = node->getCases();
  for (SwitchCase *c : cases) {
    cur_cfg->mergeCase(getInnerCFG(c), cur_cfgnode);
  }
  addInnerCFG(node, cur_cfg);
}
void CFGBuilder::visit(v2::CaseStmt *node) {
  pre(node);
  Visitor::visit(node);
  createCurrent(node);
  vector<Stmt*> stmts = node->getBody();
  for (Stmt *stmt : stmts) {
    cur_cfg->merge(getInnerCFG(stmt));
  }
  addInnerCFG(node, cur_cfg);
}
void CFGBuilder::visit(v2::DefaultStmt *node) {
  pre(node);
  Visitor::visit(node);
  createCurrent(node);
  vector<Stmt*> stmts = node->getBody();
  for (Stmt *stmt : stmts) {
    cur_cfg->merge(getInnerCFG(stmt));
  }
  addInnerCFG(node, cur_cfg);
}
void CFGBuilder::visit(v2::ForStmt *node) {
  pre(node);
  Visitor::visit(node);
  createCurrent(node);
  Stmt *body = node->getBody();
  cur_cfg->merge(getInnerCFG(body));
  // TODO back edge
  addInnerCFG(node, cur_cfg);
}
void CFGBuilder::visit(v2::WhileStmt *node) {
  pre(node);
  Visitor::visit(node);
  createCurrent(node);
  Stmt *body = node->getBody();
  cur_cfg->merge(getInnerCFG(body));
  // TODO back edge
  addInnerCFG(node, cur_cfg);
}
void CFGBuilder::visit(v2::DoStmt *node) {
  pre(node);
  Visitor::visit(node);
  createCurrent(node);
  Stmt *body = node->getBody();
  cur_cfg->merge(getInnerCFG(body));
  // TODO back edge
  addInnerCFG(node, cur_cfg);
}
void CFGBuilder::visit(v2::BreakStmt *node) {
  pre(node);
  Visitor::visit(node);
  createCurrent(node);
  addInnerCFG(node, cur_cfg);
}
void CFGBuilder::visit(v2::ContinueStmt *node) {
  pre(node);
  Visitor::visit(node);
  createCurrent(node);
  addInnerCFG(node, cur_cfg);
}
void CFGBuilder::visit(v2::ReturnStmt *node) {
  pre(node);
  Visitor::visit(node);
  createCurrent(node);
  addInnerCFG(node, cur_cfg);
}
void CFGBuilder::visit(v2::Expr *node) {
  pre(node);
}
void CFGBuilder::visit(v2::DeclStmt *node) {
  pre(node);
  Visitor::visit(node);
  createCurrent(node);
  addInnerCFG(node, cur_cfg);
}
void CFGBuilder::visit(v2::ExprStmt *node) {
  pre(node);
  Visitor::visit(node);
  createCurrent(node);
  addInnerCFG(node, cur_cfg);
}


std::string CFGNode::getLabel() {
  std::ostringstream ss;
  astnode->dump(ss);
  return ss.str();
}


std::string CFG::visualize() {
  DotGraph graph;
  std::map<CFGNode*, int> IDs;
  int ID=0;
  for (CFGNode* node : allNodes) {
    assert(node);
    graph.AddNode(std::to_string(ID), node->getLabel());
    IDs[node]=ID;
    ID++;
  }
  for (auto m : edges) {
    graph.AddEdge(std::to_string(IDs[m.first]),
                  std::to_string(IDs[m.second]));
  }
  std::string dotstring = graph.dump();
  return visualize_dot_graph(dotstring);
}


void CFG::merge(CFG *cfg) {
  assert(cfg);
  // connect this end to cfg's entry
  std::set<CFGNode*> ins = cfg->getIns();
  std::set<CFGNode*> outs = cfg->getOuts();
  for (CFGNode *from : OUTs) {
    for (CFGNode *to : ins) {
      addEdge(from, to);
    }
  }
  setOut(outs);
  addNodes(cfg->getAllNodes());
  addEdges(cfg->getAllEdges());
}
void CFG::mergeBranch(CFG *cfg, CFGNode*node, bool b) {
  std::set<CFGNode*> ins = cfg->getIns();
  std::set<CFGNode*> outs = cfg->getOuts();
  std::cout << "Merging " << (b?"true":"false") << " branch" << "\n";
  std::cout << "Edges: " << edges.size() << "\n";
  for (CFGNode *n : ins) {
    std::cout << "Adding an edge" << "\n";
    addEdge(node, n);
  }
  for (CFGNode *n : outs) {
    addOut(n);
  }
  std::cout << "Edges after: " << edges.size() << "\n";
  addNodes(cfg->getAllNodes());
  addEdges(cfg->getAllEdges());
}
void CFG::mergeCase(CFG *cfg, CFGNode *node) {
  std::set<CFGNode*> ins = cfg->getIns();
  std::set<CFGNode*> outs = cfg->getOuts();
  for (CFGNode *n : ins) {
    addEdge(node, n);
  }
  for (CFGNode *n : outs) {
    addOut(n);
  }
  addNodes(cfg->getAllNodes());
  addEdges(cfg->getAllEdges());
}
