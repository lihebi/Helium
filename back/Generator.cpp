#include "helium/parser/Visitor.h"
#include "helium/parser/AST.h"
#include "helium/parser/SourceManager.h"
#include "helium/utils/StringUtils.h"

#include "helium/type/Type.h"
#include <iostream>

using std::vector;
using std::string;
using std::map;
using std::set;



void Generator::outputInstrument(ASTNodeBase *node) {
  if (OutputPosition == node) {
    Prog += "// Output Position\n";
    for (auto &m : OutputInstrument) {
      std::string var = m.first;
      ASTNodeBase *def = m.second;
      std::map<std::string, std::string> vars = def->getFullVars();
      if (vars.count(var) == 1) {
        
        sum_output_var++;
        
        std::string type = vars[var];
        Type *t = TypeFactory::CreateType(type);
        if (dynamic_cast<PrimitiveType*>(t)) {
          
          sum_used_output_var++;
          
          std::string code = t->GetOutputCode(var);
          Prog += code + "\n";
        } else {
          Prog += "// output not used: " + var + " of type " + type + "\n";
        }
      }
    }
  }
  // write the summary to a file
}


// high level
void Generator::visit(TokenNode *node){
  if (selection.count(node) == 1) {
    Prog += node->getText() + " ";
  }
  outputInstrument(node);
}
void Generator::visit(TranslationUnitDecl *node){
  std::vector<ASTNodeBase*> nodes = node->getDecls();
  Visitor::visit(node);
  outputInstrument(node);
}
void Generator::visit(FunctionDecl *node){
  TokenNode *ReturnNode = node->getReturnTypeNode();
  // output location information
  if (selection.count(ReturnNode)==1) {
    Prog += "// " + node->getASTContext()->getFileName() + ":"
      + std::to_string(node->getBeginLoc().getLine()) + "\n";
  }
  if (ReturnNode) ReturnNode->accept(this);
  Prog += " ";

  // TRICK: do a trick here: If the function is main, replace the name
  // to helium_main there must be only main function right.. unless
  // the code contains many exectuables .. That is not interesting

  TokenNode *NameNode = node->getNameNode();
  if (NameNode) {
    // NameNode->accept(this);
    if (selection.count(NameNode) == 1) {
      // only if this function is selected will do the trick,
      // otherwise I will get a lot of helium_main
      if (NameNode->getText() == "main") {
        Prog += "helium_main";
      } else {
        NameNode->accept(this);
      }
      Prog += "(";
    }
  }
  // param node should handle parenthesis
  TokenNode *ParamNode = node->getParamNode();
  if (ParamNode) {
    ParamNode->accept(this);
    if (InputVarNodes.count(ParamNode)==1) {
      // TODO one param is an input
    }
  }

  if (NameNode && selection.count(NameNode) == 1) {
    Prog += ")";
  }
  // compound should handle curly braces
  Stmt *body = node->getBody();
  if (body) body->accept(this);
  outputInstrument(node);
}
void Generator::visit(CompoundStmt *node){
  Visitor::visit(node);
  outputInstrument(node);
}
// condition
void Generator::visit(IfStmt *node){
  TokenNode *IfNode = node->getIfNode();
  assert(IfNode);
  if (selection.count(IfNode)==1) {
    Prog += "// " + node->getASTContext()->getFileName() + ":"
      + std::to_string(node->getBeginLoc().getLine()) + "\n";
  }
  IfNode->accept(this);
  if (selection.count(IfNode)==1) Prog += "(";
  Expr *expr = node->getCond();
  if (expr) expr->accept(this);
  if (selection.count(IfNode)==1) Prog += ")";

  // I'm adding these parenthesis back because
  // if () char a; will not compile
  if (selection.count(IfNode) == 1) {Prog += "{";}
  Stmt *then_stmt = node->getThen();
  if (then_stmt) then_stmt->accept(this);
  if (selection.count(IfNode) == 1) {Prog += "}";}

  TokenNode *ElseNode = node->getElseNode();
  if (ElseNode) ElseNode->accept(this);


  if (selection.count(ElseNode) == 1) {Prog += "{";}
  Stmt *else_stmt = node->getElse();
  if (else_stmt) else_stmt->accept(this);
  if (selection.count(ElseNode) == 1) {Prog += "}";}
  outputInstrument(node);
}
void Generator::visit(SwitchStmt *node){
  TokenNode *SwitchNode = node->getSwitchNode();
  assert(SwitchNode);

  if (selection.count(SwitchNode)==1) {
    Prog += "// " + node->getASTContext()->getFileName() + ":"
      + std::to_string(node->getBeginLoc().getLine()) + "\n";
  }
  SwitchNode->accept(this);
  if (selection.count(SwitchNode) == 1) Prog += "(";
  Expr *cond = node->getCond();
  if (cond) cond->accept(this);
  if (selection.count(SwitchNode) == 1) Prog += ")";
  if (selection.count(SwitchNode) == 1) Prog += "{\n";
  std::vector<SwitchCase*> cases = node->getCases();
  for (SwitchCase *c : cases) {
    if (c) c->accept(this);
  }
  if (selection.count(SwitchNode) == 1) Prog += "}\n";
  outputInstrument(node);
}
void Generator::visit(CaseStmt *node){
  TokenNode *token = node->getCaseNode();
  if (token) token->accept(this);
  Expr *cond = node->getCond();
  if (cond) cond->accept(this);
  if (selection.count(token) == 1) {
    // HACK also add an empty statement because:
    // error: label at end of compound statement: expected statement
    Prog += ": ;";
  }
  vector<Stmt*> body = node->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
  outputInstrument(node);
}
void Generator::visit(DefaultStmt *node){
  TokenNode *token = node->getDefaultNode();
  if (token) token->accept(this);
  if (selection.count(token) == 1) {
    // HACK also add an empty statement because:
    // error: label at end of compound statement: expected statement
    Prog += ": ;";
  }
  vector<Stmt*> body = node->getBody();
  for (Stmt *stmt : body) {
    if (stmt) stmt->accept(this);
  }
  outputInstrument(node);
}
// loop
void Generator::visit(ForStmt *node){
  TokenNode *ForNode = node->getForNode();
  assert(ForNode);
  ForNode->accept(this);
  if (selection.count(ForNode) == 1) Prog += "(";
  Expr *init = node->getInit();
  if (init) init->accept(this);
  if (selection.count(ForNode) == 1) Prog += ";";
  Expr *cond = node->getCond();
  if (cond) cond->accept(this);
  if (selection.count(ForNode) == 1) Prog += ";";
  Expr *inc = node->getInc();
  if (inc) inc->accept(this);
  if (selection.count(ForNode) == 1) Prog += ") ";

  if (selection.count(ForNode) == 1) Prog += "{";

  Stmt *body = node->getBody();
  if (body) body->accept(this);

  if (selection.count(ForNode) == 1) Prog += "}";
  outputInstrument(node);
}
void Generator::visit(WhileStmt *node){
  TokenNode *WhileNode = node->getWhileNode();
  if (WhileNode) WhileNode->accept(this);
  if (selection.count(WhileNode) == 1) Prog += "(";
  Expr *cond = node->getCond();
  if (cond) cond->accept(this);
  if (selection.count(WhileNode) == 1) Prog += ") ";
  if (selection.count(WhileNode) == 1) Prog += "{";
  Stmt *body = node->getBody();
  if (body) body->accept(this);
  if (selection.count(WhileNode) == 1) Prog += "}";
  outputInstrument(node);
}
void Generator::visit(DoStmt *node){
  TokenNode *DoNode = node->getDoNode();
  TokenNode *WhileNode = node->getWhileNode();
  assert(DoNode);
  assert(WhileNode);
  DoNode->accept(this);
  Stmt *body = node->getBody();
  if (body) body->accept(this);
  WhileNode->accept(this);
  if (selection.count(WhileNode) == 1) {Prog += "(";}
  Expr *cond = node->getCond();
  if (cond) cond->accept(this);
  if (selection.count(WhileNode) == 1) {Prog += ");";}
  outputInstrument(node);
}
// single
void Generator::visit(BreakStmt *node){
  if (selection.count(node)) {
    Prog += "break;\n";
  }
  outputInstrument(node);
}
void Generator::visit(ContinueStmt *node){
  if (selection.count(node)) {
    Prog += "continue;\n";
  }
  outputInstrument(node);
}
void Generator::visit(ReturnStmt *node){
  TokenNode *ReturnNode = node->getReturnNode();
  if (ReturnNode) {
    // if adjust return, don't output the return
    if (!AdjustReturn) {
      ReturnNode->accept(this);
    }
  }
  Expr *expr = node->getValue();
  if (expr) expr->accept(this);
  if (selection.count(ReturnNode) == 1) {
    Prog += ";\n";
  }
  outputInstrument(node);
}
// expr stmt
void Generator::visit(Expr *node){
  if (selection.count(node) == 1) {
    Prog += node->getText();
  }
  outputInstrument(node);
}
void Generator::visit(DeclStmt *node){
  if (selection.count(node) == 1) {
    Prog += node->getText() + "\n";

    if (InputVarNodes.count(node)==1) {
      // The variable is input variable
      // generate input code
      // 1. get the type and the name of the variable
      std::map<std::string, std::string> fullVars = node->getFullVars();
      // 2. if it is primitive type, read from the input file
      for (auto &m : fullVars) {
        
        sum_input_var++;
        
        Type *t = TypeFactory::CreateType(m.second);
        if (dynamic_cast<PrimitiveType*>(t)) {

          sum_used_input_var++;
          
          std::string code = t->GetInputCode(m.first);
          Prog += "// Input code for " + m.first + " of type " + m.second + "\n";
          Prog += code + "\n";
        } else {
          Prog += "// TODO input not used: " + m.first + " of type " + m.second + "\n";
        }
      }
    }
  }
  outputInstrument(node);
}
void Generator::visit(ExprStmt *node){
  if (selection.count(node) == 1) {
    // no need semi-colon because <expr_stmt>... ;</expr_stmt>
    Prog += "// " + node->getASTContext()->getFileName() + ":"
      + std::to_string(node->getBeginLoc().getLine()) + "\n";
    Prog += node->getText() + "\n";
  }
  outputInstrument(node);
}
