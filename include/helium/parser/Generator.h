#ifndef GENERATOR_H
#define GENERATOR_H

#include "Visitor.h"

/**
 * Code Generator
 */
class Generator : public Visitor {
public:
  Generator() {}
  virtual ~Generator() {}
  // high level
  virtual void visit(TokenNode *node);
  virtual void visit(TranslationUnitDecl *node);
  virtual void visit(FunctionDecl *node);
  virtual void visit(CompoundStmt *node);
  // condition
  virtual void visit(IfStmt *node);
  virtual void visit(SwitchStmt *node);
  virtual void visit(CaseStmt *node);
  virtual void visit(DefaultStmt *node);
  // loop
  virtual void visit(ForStmt *node);
  virtual void visit(WhileStmt *node);
  virtual void visit(DoStmt *node);
  // single
  virtual void visit(BreakStmt *node);
  virtual void visit(ContinueStmt *node);
  virtual void visit(ReturnStmt *node);
  // expr stmt
  virtual void visit(Expr *node);
  virtual void visit(DeclStmt *node);
  virtual void visit(ExprStmt *node);


  void setSelection(std::set<ASTNodeBase*> sel) {
    selection = sel;
  }

  void setInputVarNodes(std::set<ASTNodeBase*> nodes) {
    InputVarNodes = nodes;
  }

  std::string getProgram() {
    return Prog;
  }
  void adjustReturn(bool b) {
    AdjustReturn = b;
  }

  void setOutputInstrument(std::map<std::string, ASTNodeBase*> toinstrument, ASTNodeBase *last, SymbolTable symtbl) {
    OutputInstrument = toinstrument;
    OutputPosition = last;
    OutputPositionSymtbl = symtbl;
  }
  void outputInstrument(ASTNodeBase *node);
  std::vector<int> getIOSummary() {
    return {sum_output_var, sum_used_output_var, sum_input_var, sum_used_input_var};
  }
private:
  std::string Prog;
  std::set<ASTNodeBase*> selection;
  bool AdjustReturn = false;
  std::set<ASTNodeBase*> InputVarNodes;
  std::map<std::string, ASTNodeBase*> OutputInstrument;
  ASTNodeBase *OutputPosition = nullptr;
  SymbolTable OutputPositionSymtbl;
  // summary about output instrumentation
  int sum_output_var=0;
  int sum_used_output_var=0;
  // summary about input instrumentation
  int sum_input_var=0;
  int sum_used_input_var=0;
};



#endif /* GENERATOR_H */
