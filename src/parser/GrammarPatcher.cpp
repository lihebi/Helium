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

void StandAloneGrammarPatcher::process() {
  // std::cout << "StandAloneGrammarPatcher::process" << "\n";
  // get the lowest level nodes
  LevelVisitor *levelVisitor = new LevelVisitor();
  TranslationUnitDecl *unit = AST->getTranslationUnitDecl();
  unit->accept(levelVisitor);

  // std::cout << "Level Visitor Result:" << "\n";
  // std::map<v2::ASTNodeBase*, int> levels = levelVisitor->getLevels();
  // for (auto &m : levels) {
  //   std::cout << m.second << "\n";
  // }

  // first, clean up the selection: remove those not in this AST
  // otherwise there will be infinite loop in worklist
  for (ASTNodeBase *node : selection) {
    if (levelVisitor->getLevel(node) != -1) {
      worklist.insert(node);
    }
  }
  patch = worklist; // this is the result

  while (!worklist.empty()) {
    // std::cout << "Worklist size: " << worklist.size() << "\n";
    // print out what is inside worklist
    // for (ASTNodeBase *node : worklist) {
    //   node->dump(std::cout);
    //   std::cout << " On level " << levelVisitor->getLevel(node) << "\n";
    // }
    
    ASTNodeBase *node = levelVisitor->getLowestLevelNode(worklist);
    if (!node) continue;
    worklist.erase(node);
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
        if (selection.count(c) == 1) {
          siblings.insert(c);
          worklist.erase(c);
        }
      }
      siblings.insert(node);
      // patch = matchMin(parent, sel)
      std::set<ASTNodeBase*> p = matchMin(parent, siblings);
      // ret.add(patch)
      patch.insert(p.begin(), p.end());
      // worklist.add(parent);
      worklist.insert(parent);
    }
  }
}



std::set<ASTNodeBase*> StandAloneGrammarPatcher::matchMin(v2::ASTNodeBase *parent, std::set<v2::ASTNodeBase*> sel) {
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
  GrammarPatcher grammarPatcher;
  PatchData data;
  data.selection = sel;
  if (parent) parent->accept(&grammarPatcher, &data);
  return grammarPatcher.getPatch();
}




// TODO
void GrammarPatcher::visit(v2::TokenNode *token, void *data) {}
void GrammarPatcher::visit(v2::TranslationUnitDecl *unit, void *data) {}
void GrammarPatcher::visit(v2::FunctionDecl *function, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  // select the whole thing because I need to keep the signature of the function
  TokenNode *token = function->getReturnTypeNode();
  if (token) patch.insert(token);
  token = function->getNameNode();
  if (token) patch.insert(token);
  token = function->getParamNode();
  if (token) patch.insert(token);
  Stmt *body = function->getBody();
  // body is special.
  if (body) {
    if (selection.count(body) == 0) {
      // no selection data is passed in. Treat as no selection.
      patch.insert(body);
      body->accept(this);
    }
  }
}
void GrammarPatcher::visit(v2::DeclStmt *decl_stmt, void *data) {}
void GrammarPatcher::visit(v2::ExprStmt *expr_stmt, void *data) {}
/**
 * CompoundStmt ::= stmt*
 * No need to select anything
 */
void GrammarPatcher::visit(v2::CompoundStmt *comp_stmt, void *data) {
  if (TokenNode *CompNode = comp_stmt->getCompNode()) {
    patch.insert(CompNode);
  }
}
void GrammarPatcher::visit(v2::ForStmt *for_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *token = for_stmt->getForNode();
  if (token) {
    if (selection.count(token) == 0) {
      patch.insert(token);
    }
  }
  // no need these
  // Expr *init = for_stmt->getInit();
  // Expr *cond = for_stmt->getCond();
  // Expr *inc = for_stmt->getInc();
  Stmt *body = for_stmt->getBody();
  if (body) {
    if (selection.count(body) == 0) {
      patch.insert(body);
      body->accept(this);
    }
  }
}
void GrammarPatcher::visit(v2::WhileStmt *while_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *token = while_stmt->getWhileNode();
  if (token) {
    // token->accept(this);
    patch.insert(token);
  }
  Expr *cond = while_stmt->getCond();
  if (cond) {
    patch.insert(cond);
    // cond->accept(this);
  }
  Stmt *body = while_stmt->getBody();
  if (body) {
    if (selection.count(body) == 0) {
      patch.insert(body);
      body->accept(this);
    }
  }
}
void GrammarPatcher::visit(v2::DoStmt *do_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *token = do_stmt->getDoNode();
  if (token) {
    // token->accept(this);
    patch.insert(token);
  }
  Stmt *body = do_stmt->getBody();
  if (body) {
    if (selection.count(body) == 0) {
      patch.insert(body);
      body->accept(this);
    }
  }
  token = do_stmt->getWhileNode();
  if (token) {
    patch.insert(token);
    // token->accept(this);
  }
  Expr *cond = do_stmt->getCond();
  if (cond) {
    // cond->accept(this);
    patch.insert(cond);
  }
}
void GrammarPatcher::visit(v2::BreakStmt *break_stmt, void *data) {}
void GrammarPatcher::visit(v2::ContinueStmt *cont_stmt, void *data) {}
void GrammarPatcher::visit(v2::ReturnStmt *ret_stmt, void *data) {
  TokenNode *ReturnNode = ret_stmt->getReturnNode();
  if (ReturnNode) {
    // ReturnNode->accept(this);
    patch.insert(ReturnNode);
  }
  // add value because it is related to function signature
  Expr *value = ret_stmt->getValue();
  if (value) {
    // value->accept(this);
    patch.insert(value);
  }
}
void GrammarPatcher::visit(v2::IfStmt *if_stmt, void *data) {
  std::set<ASTNodeBase*> selection;
  if (data) selection = static_cast<PatchData*>(data)->selection;
  TokenNode *token = if_stmt->getIfNode();
  if (token) {
    // token->accept(this);
    patch.insert(token);
  }
  Expr *expr = if_stmt->getCond();
  if (expr) {
    patch.insert(expr);
    // expr->accept(this);
  }
  Stmt *then_stmt = if_stmt->getThen();
  assert(then_stmt);
  // must have the then
  if (then_stmt) {
    // if (selection.count(then_stmt) == 0) {
    patch.insert(then_stmt);
    then_stmt->accept(this);
    // }
  }
  TokenNode *ElseNode = if_stmt->getElseNode();
  Stmt *else_stmt = if_stmt->getElse();
  if (selection.count(ElseNode) == 1 || selection.count(else_stmt) == 1) {
    if (ElseNode) {
      patch.insert(ElseNode);
    }
    if (else_stmt) {
      patch.insert(else_stmt);
      else_stmt->accept(this);
    }
  }
}
void GrammarPatcher::visit(v2::SwitchStmt *switch_stmt, void *data) {
  TokenNode *token = switch_stmt->getSwitchNode();
  if (token) {
    patch.insert(token);
    // token->accept(this);
  }
  Expr *cond = switch_stmt->getCond();
  if (cond) {
    // cond->accept(this);
    patch.insert(cond);
  }
  // no need for cases
  // std::vector<SwitchCase*> cases = switch_stmt->getCases();
  // for (SwitchCase *c : cases) {
  //   if (c) c->accept(this);
  // }
}
void GrammarPatcher::visit(v2::CaseStmt *case_stmt, void *data) {
  TokenNode *token = case_stmt->getCaseNode();
  if (token) {
    patch.insert(token);
  }
  Expr *cond = case_stmt->getCond();
  if (cond) {
    patch.insert(cond);
  }
  // no need body
  // vector<Stmt*> body = case_stmt->getBody();
  // for (Stmt *stmt : body) {
  //   if (stmt) stmt->accept(this);
  // }
}
void GrammarPatcher::visit(v2::DefaultStmt *def_stmt, void *data) {
  TokenNode *token = def_stmt->getDefaultNode();
  if (token) {
    patch.insert(token);
  }
}
void GrammarPatcher::visit(v2::Expr *expr, void *data) {}

