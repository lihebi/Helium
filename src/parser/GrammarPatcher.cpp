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
  // std::cout << "StandAloneGrammarPatcher::process" << "\n";
  // get the lowest level nodes
  LevelVisitorV2 *levelVisitor = new LevelVisitorV2();
  TranslationUnitDecl *unit = AST->getTranslationUnitDecl();
  unit->accept(levelVisitor);

  // std::cout << "Level Visitor Result:" << "\n";
  // std::map<v2::ASTNodeBase*, int> levels = levelVisitor->getLevels();
  // for (auto &m : levels) {
  //   m.first->dump(std::cout);
  //   std::cout << " " << m.second << "\n";
  // }

  // first, clean up the selection: remove those not in this AST
  // otherwise there will be infinite loop in worklist
  for (ASTNodeBase *node : Selection) {
    if (levelVisitor->getLevel(node) != -1) {
      Worklist.insert(node);
    }
  }
  Patch = Worklist; // this is the result

  while (!Worklist.empty()) {
    // DEBUG
    // std::cout << "Worklist: ";
    // for (ASTNodeBase *node : Worklist) {
    //   node->dump(std::cout);
    // }
    // std::cout << "\n";
    
    // std::cout << "Worklist size: " << worklist.size() << "\n";
    // print out what is inside worklist
    // for (ASTNodeBase *node : worklist) {
    //   node->dump(std::cout);
    //   std::cout << " On level " << levelVisitor->getLevel(node) << "\n";
    // }

    ASTNodeBase *node = levelVisitor->getLowestLevelNode(Worklist);
    if (!node) continue;
    Worklist.erase(node);
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
      set<ASTNodeBase*> children = parentIndexer->getChildren(parent);
      set<ASTNodeBase*> siblings;
      for (auto *c : children) {
        if (Worklist.count(c) == 1) {
          siblings.insert(c);
          Worklist.erase(c);
        }
      }
      siblings.insert(node);

      // DEBUG
      // std::cout << "Worklist: ";
      // for (ASTNodeBase *node : Worklist) {
      //   node->dump(std::cout);
      // }
      // std::cout << "\n";
      
      matchMin(parent, siblings);

      // this parent is valid by it own, aka statement
      if (Worklist.empty() && validAlone(parent)) {
        break;
      }
      Worklist.insert(parent);
    }
  }
  // remove skip from patch
  // for (auto m : GlobalSkip) {
  //   Patch.erase(m.first);
  // }
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

  GrammarPatcher grammarPatcher;
  PatchData data;
  data.selection = sel;
  parent->accept(&grammarPatcher, &data);
  std::set<v2::ASTNodeBase*> patch = grammarPatcher.getPatch();
  this->Patch.insert(patch.begin(), patch.end());
}




// TODO
void GrammarPatcher::visit(v2::TokenNode *token, void *data) {}
void GrammarPatcher::visit(v2::TranslationUnitDecl *unit, void *data) {}
void GrammarPatcher::visit(v2::FunctionDecl *function, void *data) {
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
  
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  // select the whole thing because I need to keep the signature of the function
  TokenNode *ReturnTypeNode = function->getReturnTypeNode();
  assert(ReturnTypeNode);
  TokenNode *NameNode = function->getNameNode();
  assert(NameNode);
  TokenNode *ParamNode = function->getParamNode();
  assert(ParamNode);
  Stmt *body = function->getBody();
  assert(body);
  // body is special.
  if (selection.size() == 1 && selection.count(body)==1) {
    Patch.insert(body);
    body->accept(this);
    GlobalSkip[function] = body;
  } else {
    // no selection data is passed in. Treat as no selection.
    Patch.insert(ReturnTypeNode);
    Patch.insert(NameNode);
    Patch.insert(ParamNode);
    Patch.insert(body);
    body->accept(this);
  }
}
void GrammarPatcher::visit(v2::DeclStmt *decl_stmt, void *data) {}
void GrammarPatcher::visit(v2::ExprStmt *expr_stmt, void *data) {}
/**
 * CompoundStmt ::= stmt*
 * No need to select anything
 */
void GrammarPatcher::visit(v2::CompoundStmt *comp_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *CompNode = comp_stmt->getCompNode();
  assert(CompNode);
  if (selection.size() == 0) {
    // this should serve as a stop point for many matchMin
    Patch.insert(CompNode);
  } else if (selection.size() == 1) {
    // lazy evaluation & replacement
    // FIXME verify the first in selection is a body statement
    GlobalSkip[comp_stmt] = *selection.begin();
  } else {
    // this actually should not happen
    // assert(false);
    Patch.insert(CompNode);
  }
}
void GrammarPatcher::visit(v2::ForStmt *for_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *ForNode = for_stmt->getForNode();
  assert(ForNode);
  Stmt *body = for_stmt->getBody();
  assert(body);

  if (selection.size() == 1 && selection.count(body) ==1) {
    Patch.insert(body);
    body->accept(this);
    GlobalSkip[for_stmt] = body;
  } else {
    Patch.insert(ForNode);
    // didn't patch init,condition,inc because they are optional
    Patch.insert(body);
    body->accept(this);
  }
}
void GrammarPatcher::visit(v2::WhileStmt *while_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *WhileNode = while_stmt->getWhileNode();
  assert(WhileNode);
  Stmt *body = while_stmt->getBody();
  assert(body);
  Expr *cond = while_stmt->getCond();
  assert(cond);

  if (selection.size() == 1 && selection.count(body) == 1) {
    GlobalSkip[while_stmt] = body;
  } else {
    Patch.insert(WhileNode);
    Patch.insert(cond);
    Patch.insert(body);
    body->accept(this);
  }
}
void GrammarPatcher::visit(v2::DoStmt *do_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *DoNode = do_stmt->getDoNode();
  assert(DoNode);
  Stmt *body = do_stmt->getBody();
  assert(body);
  TokenNode *WhileNode = do_stmt->getWhileNode();
  assert(WhileNode);
  Expr *cond = do_stmt->getCond();
  assert(cond);
  if (selection.size() == 1 && selection.count(body) == 1) {
    GlobalSkip[do_stmt] = body;
  } else {
    Patch.insert(DoNode);
    Patch.insert(WhileNode);
    Patch.insert(cond);
    if (selection.count(body) == 0) {
      Patch.insert(body);
      body->accept(this);
    }
  }
}
void GrammarPatcher::visit(v2::BreakStmt *break_stmt, void *data) {}
void GrammarPatcher::visit(v2::ContinueStmt *cont_stmt, void *data) {}
void GrammarPatcher::visit(v2::ReturnStmt *ret_stmt, void *data) {
  TokenNode *ReturnNode = ret_stmt->getReturnNode();
  assert(ReturnNode);
  Patch.insert(ReturnNode);
  // add value because it is related to function signature
  Expr *value = ret_stmt->getValue();
  if (value) {
    // value->accept(this);
    Patch.insert(value);
  }
}
void GrammarPatcher::visit(v2::IfStmt *if_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *IfNode = if_stmt->getIfNode();
  assert(IfNode);
  Expr *cond = if_stmt->getCond();
  Stmt *then_stmt = if_stmt->getThen();
  assert(then_stmt);
  TokenNode *ElseNode = if_stmt->getElseNode();
  Stmt *else_stmt = if_stmt->getElse();


  if (selection.size() == 1 && selection.count(then_stmt) == 1) {
    GlobalSkip[if_stmt] = then_stmt;
  } else if (selection.size() == 1 && selection.count(else_stmt) == 1) {
    GlobalSkip[if_stmt] = else_stmt;
  } else {
    Patch.insert(IfNode);
    Patch.insert(cond);
    // must have the then
    Patch.insert(then_stmt);
    then_stmt->accept(this);
    if (selection.count(ElseNode) == 1 || selection.count(else_stmt) == 1) {
      assert(ElseNode);
      assert(else_stmt);
      Patch.insert(ElseNode);
      Patch.insert(else_stmt);
      else_stmt->accept(this);
    }
  }
}

void GrammarPatcher::visit(v2::SwitchStmt *switch_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *SwitchNode = switch_stmt->getSwitchNode();
  assert(SwitchNode);
  std::vector<SwitchCase*> cases = switch_stmt->getCases();
  std::set<ASTNodeBase*> case_set(cases.begin(), cases.end());
  Expr *cond = switch_stmt->getCond();
  assert(cond);

  // if more than one case is selected, switch is necessary
  if (selection.size() == 1
      && case_set.count(*selection.begin()) == 1
      && GlobalSkip.count(*selection.begin()) == 1) {
    GlobalSkip[switch_stmt] = *selection.begin();
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
void GrammarPatcher::visit(v2::CaseStmt *case_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *CaseNode = case_stmt->getCaseNode();
  Expr *cond = case_stmt->getCond();
  assert(CaseNode);
  assert(cond);
  if (selection.count(CaseNode) == 0) {
    // FIXME this nullptr because in AST it maintain a list of statements
    // DO NOT USE THIS VALUE
    GlobalSkip[case_stmt] = nullptr;
  }
  Patch.insert(CaseNode);
  Patch.insert(cond);
  // no need body
  // vector<Stmt*> body = case_stmt->getBody();
  // for (Stmt *stmt : body) {
  //   if (stmt) stmt->accept(this);
  // }
}
void GrammarPatcher::visit(v2::DefaultStmt *def_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *DefaultNode = def_stmt->getDefaultNode();
  if (selection.count(DefaultNode) == 0) {
    // FIXME this nullptr because in AST it maintain a list of statements
    // DO NOT USE THIS VALUE
    GlobalSkip[def_stmt] = nullptr;
  }
  assert(DefaultNode);
  Patch.insert(DefaultNode);
}
void GrammarPatcher::visit(v2::Expr *expr, void *data) {}

