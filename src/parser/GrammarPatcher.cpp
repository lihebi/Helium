#include "helium/parser/GrammarPatcher.h"
#include "helium/parser/visitor.h"

#include "helium/parser/ast_v2.h"
#include "helium/parser/source_manager.h"
#include "helium/utils/string_utils.h"
#include <iostream>

using std::vector;
using std::string;
using std::map;
using std::set;

using namespace v2;


// the skip map for lazy evaluation
std::map<v2::ASTNodeBase*, v2::ASTNodeBase*> GlobalSkip;

void StandAloneGrammarPatcher::process() {
  GlobalSkip.clear();
  // std::cout << "StandAloneGrammarPatcher::process" << "\n";
  // get the lowest level nodes
  LevelVisitor *levelVisitor = new LevelVisitor();
  TranslationUnitDecl *unit = AST->getTranslationUnitDecl();
  unit->accept(levelVisitor);

  // first, clean up the selection: remove those not in this AST
  // otherwise there will be infinite loop in worklist
  for (ASTNodeBase *node : Selection) {
    if (levelVisitor->getLevel(node) != -1) {
      Worklist.insert(node);
    }
  }
  Patch = Worklist; // this is the result

  while (!Worklist.empty()) {

    ASTNodeBase *node = levelVisitor->getLowestLevelNode(Worklist);
    if (!node) continue;
    Worklist.erase(node);
    if (Worklist.empty() && validAlone(node)) break;
    // if this is the only one node left, we complete itself, and
    // stop!  this is not necessary when the node is popped up along
    // parents but it is necessary if only one node is originally
    // selected.
    //
    // FIXME this is buggy. The one selected might be a condition
    // expression, itself cannot make a valid program statement. Also,
    // When several branches merge, the parent might still no a valid
    // statement.
    // if (Worklist.empty() && validAlone(node)) {
    //   matchMin(node, {});
    //   break;
    // }
    
    ParentIndexer *parentIndexer = new ParentIndexer();
    unit->accept(parentIndexer);
    // for one of them, get all the siblings
    // remove them from worklist, and process
    // parent = getparent(sel)
    ASTNodeBase *parent = parentIndexer->getParent(node);
    if (parent) {
      vector<ASTNodeBase*> children = parentIndexer->getChildren(parent);
      set<ASTNodeBase*> siblings;
      for (auto *c : children) {
        if (Worklist.count(c) == 1) {
          siblings.insert(c);
          Worklist.erase(c);
        }
      }
      siblings.insert(node);
      
      matchMin(parent, siblings);

      Patch.insert(parent);
      Worklist.insert(parent);
    }
  }
  // remove skip from patch
  for (auto m : GlobalSkip) {
    Patch.erase(m.first);
  }
}
bool StandAloneGrammarPatcher::validAlone(v2::ASTNodeBase* node) {
  if (dynamic_cast<v2::TokenNode*>(node)
      || dynamic_cast<v2::CaseStmt*>(node)
      || dynamic_cast<v2::DefaultStmt*>(node)
      || dynamic_cast<v2::Expr*>(node)) return false;
  else return true;
}

void StandAloneGrammarPatcher::matchMin(v2::ASTNodeBase *parent, std::set<v2::ASTNodeBase*> sel) {
  // get rules
  // for each rule
  // T=body, S=children
  // seqs = subseq(T,S)
  // for seq in seqs
  // if (sel \in seq) we found the valid
  // expand = matchMin(extra, \empty)
  // return min(patch)

  // I'm actually going to implement this closely with each AST node
  // GrammarPatcher grammarPatcher(parent, sel);
  assert(parent);


  // DEBUG
  // std::cout << "current matchMin: " << "\n";
  // std::cout << "parent: ";
  // parent->dump(std::cout);
  // std::cout << "\n";
  // std::cout << "children: ";
  // for (ASTNodeBase *base : sel) {
  //   base->dump(std::cout);
  // }
  // std::cout << "\n";

  GrammarPatcher patcher;
  PatchData data;
  data.selection = sel;
  patcher.setSelection(sel);
  parent->accept(&patcher);
  // patcher.patch(parent);
  std::set<v2::ASTNodeBase*> patch = patcher.getPatch();
  this->Patch.insert(patch.begin(), patch.end());

  // DEBUG
  // std::cout << "Now patch includes:" << "\n";
  // for (auto *node : Patch) {
  //   node->dump(std::cout);
  // }
  // std::cout << "\n";
  // std::cout << "Glboal Skip:" << "\n";
  // for (auto &m : GlobalSkip) {
  //   m.first->dump(std::cout);
  // }
  // std::cout << "\n";
}





// high level
void GrammarPatcher::visit(v2::TokenNode *node) {
}
void GrammarPatcher::visit(v2::TranslationUnitDecl *node) {
}
void GrammarPatcher::visit(v2::FunctionDecl *node) {
  if (GlobalSkip.count(node)==1) return;
  // if function header is selected, it is hard
  // i have to generate a main function, and generate parameters, initialize them, and call the function
  // that will force to have a conditional to check
  // if no function, i can simply put everything into the main function
  // so now i'm going to disable the function level selection
  //
  // UPDATE However, if i don't do that, variable resolving will have problem
  // I have to create declarations for them
  // Now for build rate experiment purpose, I only want to get the function header
  // Then i can simply put an empty main function.
  // I don't necessary call it right now though.
  //
  // Another advantage to keep the function: I can keep the function header for all the things
  // I know it will introduce the parameter, but that's the only thing,
  // and the only bad thing is that the type of parameters might make it hard to compile.
  // in terms of use, since the variable is not used, the value of them should not matter much.
  // assert(false && "Do not support function selection.");
  
  // std::set<ASTNodeBase*> selection;
  // if (data) selection = static_cast<PatchData*>(data)->selection;
  // select the whole thing because I need to keep the signature of the function
  TokenNode *ReturnTypeNode = node->getReturnTypeNode();
  assert(ReturnTypeNode);
  TokenNode *NameNode = node->getNameNode();
  assert(NameNode);
  TokenNode *ParamNode = node->getParamNode();
  assert(ParamNode);
  Stmt *body = node->getBody();
  assert(body);
  // body is special.
  if (Selection.size() == 1 && Selection.count(body)==1) {
    Patch.insert(body);

    GrammarPatcher patcher;
    body->accept(&patcher);
    merge(&patcher);
    
    GlobalSkip[node] = body;
  } else {
    // no selection data is passed in. Treat as no selection.
    Patch.insert(ReturnTypeNode);
    Patch.insert(NameNode);
    Patch.insert(ParamNode);
    Patch.insert(body);

    GrammarPatcher patcher;
    body->accept(&patcher);
    merge(&patcher);
  }
}
void GrammarPatcher::visit(v2::CompoundStmt *node) {
  if (GlobalSkip.count(node)==1) return;
  TokenNode *CompNode = node->getCompNode();
  assert(CompNode);
  if (Selection.size() == 0) {
    // this should serve as a stop point for many matchMin
    Patch.insert(CompNode);
  } else if (Selection.size() == 1) {
    // lazy evaluation & replacement
    // FIXME verify the first in selection is a body statement
    GlobalSkip[node] = *Selection.begin();
  } else {
    // this actually should not happen
    // assert(false);
    Patch.insert(CompNode);
  }
}
// condition
void GrammarPatcher::visit(v2::IfStmt *node) {
  if (GlobalSkip.count(node)==1) return;
  TokenNode *IfNode = node->getIfNode();
  assert(IfNode);
  Expr *cond = node->getCond();
  Stmt *then_stmt = node->getThen();
  assert(then_stmt);
  TokenNode *ElseNode = node->getElseNode();
  Stmt *else_stmt = node->getElse();


  if (Selection.size() == 1 && Selection.count(then_stmt) == 1) {
    GlobalSkip[node] = then_stmt;
  } else if (Selection.size() == 1 && Selection.count(else_stmt) == 1) {
    GlobalSkip[node] = else_stmt;
  } else {
    Patch.insert(IfNode);
    Patch.insert(cond);
    // must have the then
    Patch.insert(then_stmt);
    GrammarPatcher patcher;
    then_stmt->accept(&patcher);
    merge(&patcher);
    if (Selection.count(ElseNode) == 1 || Selection.count(else_stmt) == 1) {
      assert(ElseNode);
      assert(else_stmt);
      Patch.insert(ElseNode);
      Patch.insert(else_stmt);
      GrammarPatcher patcher;
      else_stmt->accept(&patcher);
      merge(&patcher);
    }
  }
}
void GrammarPatcher::visit(v2::SwitchStmt *node) {
  if (GlobalSkip.count(node)==1) return;
  TokenNode *SwitchNode = node->getSwitchNode();
  assert(SwitchNode);
  std::vector<SwitchCase*> cases = node->getCases();
  std::set<ASTNodeBase*> case_set(cases.begin(), cases.end());
  Expr *cond = node->getCond();
  assert(cond);

  // if more than one case is selected, switch is necessary
  if (Selection.size() == 1
      && case_set.count(*Selection.begin()) == 1
      && GlobalSkip.count(*Selection.begin()) == 1) {
    GlobalSkip[node] = *Selection.begin();
  } else {
    Patch.insert(SwitchNode);
    Patch.insert(cond);
    // FIXME should have the compound parenthesis
  }
  
  // no need for cases
  // std::vector<SwitchCase*> cases = switch_stmt->getCases();
  // for (SwitchCase *c : cases) {
  //   if (c) c->accept(this);
  // }
}
void GrammarPatcher::visit(v2::CaseStmt *node) {
  if (GlobalSkip.count(node)==1) return;
  TokenNode *CaseNode = node->getCaseNode();
  Expr *cond = node->getCond();
  assert(CaseNode);
  assert(cond);
  if (Selection.count(CaseNode) == 0) {
    // FIXME this nullptr because in AST it maintain a list of statements
    // DO NOT USE THIS VALUE
    GlobalSkip[node] = nullptr;
  }
  Patch.insert(CaseNode);
  Patch.insert(cond);
  // no need body
  // vector<Stmt*> body = case_stmt->getBody();
  // for (Stmt *stmt : body) {
  //   if (stmt) stmt->accept(this);
  // }
}
void GrammarPatcher::visit(v2::DefaultStmt *node) {
  if (GlobalSkip.count(node)==1) return;
  TokenNode *DefaultNode = node->getDefaultNode();
  if (Selection.count(DefaultNode) == 0) {
    // FIXME this nullptr because in AST it maintain a list of statements
    // DO NOT USE THIS VALUE
    GlobalSkip[node] = nullptr;
  }
  assert(DefaultNode);
  Patch.insert(DefaultNode);
}
// loop
void GrammarPatcher::visit(v2::ForStmt *node) {
  if (GlobalSkip.count(node) ==1) {
    // it is indicated to skip, do not patch it
    return;
  }
  TokenNode *ForNode = node->getForNode();
  assert(ForNode);
  Stmt *body = node->getBody();
  assert(body);

  if (Selection.size() == 1 && Selection.count(body) ==1) {
    Patch.insert(body);
    GrammarPatcher patcher;
    body->accept(&patcher);
    merge(&patcher);
    GlobalSkip[node] = body;
  } else {
    Patch.insert(ForNode);
    // didn't patch init,condition,inc because they are optional
    Patch.insert(body);
    GrammarPatcher patcher;
    body->accept(&patcher);
    merge(&patcher);
  }
}
void GrammarPatcher::visit(v2::WhileStmt *node) {
  if (GlobalSkip.count(node)==1) return;
  TokenNode *WhileNode = node->getWhileNode();
  assert(WhileNode);
  Stmt *body = node->getBody();
  assert(body);
  Expr *cond = node->getCond();
  assert(cond);

  if (Selection.size() == 1 && Selection.count(body) == 1) {
    GlobalSkip[node] = body;
  } else {
    Patch.insert(WhileNode);
    Patch.insert(cond);
    Patch.insert(body);
    GrammarPatcher patcher;
    body->accept(&patcher);
    merge(&patcher);
  }
}
void GrammarPatcher::visit(v2::DoStmt *node) {
  if (GlobalSkip.count(node)==1) return;
  TokenNode *DoNode = node->getDoNode();
  assert(DoNode);
  Stmt *body = node->getBody();
  assert(body);
  TokenNode *WhileNode = node->getWhileNode();
  assert(WhileNode);
  Expr *cond = node->getCond();
  assert(cond);
  if (Selection.size() == 1 && Selection.count(body) == 1) {
    GlobalSkip[node] = body;
  } else {
    Patch.insert(DoNode);
    Patch.insert(WhileNode);
    Patch.insert(cond);
    if (Selection.count(body) == 0) {
      Patch.insert(body);
      GrammarPatcher patcher;
      body->accept(&patcher);
      merge(&patcher);
    }
  }
}
// single
void GrammarPatcher::visit(v2::BreakStmt *node) {
  if (GlobalSkip.count(node)==1) return;
}
void GrammarPatcher::visit(v2::ContinueStmt *node) {
  if (GlobalSkip.count(node)==1) return;
}
void GrammarPatcher::visit(v2::ReturnStmt *node) {
  if (GlobalSkip.count(node)==1) return;
  TokenNode *ReturnNode = node->getReturnNode();
  assert(ReturnNode);
  Patch.insert(ReturnNode);
  // add value because it is related to function signature
  Expr *value = node->getValue();
  if (value) {
    // value->accept(this);
    Patch.insert(value);
  }
}
// expr stmt
void GrammarPatcher::visit(v2::Expr *node) {
}
void GrammarPatcher::visit(v2::DeclStmt *node) {
}
void GrammarPatcher::visit(v2::ExprStmt *node) {
}
