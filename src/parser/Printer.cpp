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



string Printer::prettyPrint(string ast) {
  // join line if ) is on a single line
  vector<string> lines = utils::split(ast, '\n');
  vector<string> ret;
  string tmp;
  for (string line : lines) {
    utils::trim(line);
    if (line.size() == 1 && line[0] == ')') {
      tmp += ')';
    } else if (line.empty()) {
      continue;
    } else {
      ret.push_back(tmp);
      tmp = line;
    }
  }
  ret.push_back(tmp);
  string retstr;
  // indent
  int indent = 0;
  for (string line : ret) {
    int open = std::count(line.begin(), line.end(), '(');
    int close = std::count(line.begin(), line.end(), ')');
    retstr += string(indent*2, ' ') + line + "\n";
    indent = indent + open - close;
  }
  return retstr;
}

/**
 * Printer
 */
void Printer::pre(v2::ASTNodeBase *node) {
  oss << "(" << node->getBeginLoc().getLine() << ":" << node->getBeginLoc().getColumn() << " "
     << node->getNodeName();
}
void Printer::post() {oss << ")";}


void Printer::visit(v2::TokenNode *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Printer::visit(v2::TranslationUnitDecl *node) {
  pre(node);
  Visitor::visit(node);
  post();oss<<"\n";
}
void Printer::visit(v2::FunctionDecl *node) {
  oss<<"\n";pre(node);oss<<"\n";
  Visitor::visit(node);
  post();oss<<"\n";
}
void Printer::visit(v2::IfStmt *node) {
  oss<<"\n";pre(node);oss<<"\n";
  Visitor::visit(node);
  post();oss<<"\n";
}
void Printer::visit(v2::SwitchStmt *node) {
  oss<<"\n";pre(node);oss<<"\n";
  Visitor::visit(node);
  post();oss<<"\n";
}
void Printer::visit(v2::CaseStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();oss<<"\n";
}
void Printer::visit(v2::DefaultStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();oss<<"\n";
}
void Printer::visit(v2::ForStmt *node) {
  oss<<"\n";pre(node);oss<<"\n";
  Visitor::visit(node);
  post();oss<<"\n";
}
void Printer::visit(v2::WhileStmt *node) {
  oss<<"\n";pre(node);oss<<"\n";
  Visitor::visit(node);
  post();oss<<"\n";
}
void Printer::visit(v2::DoStmt *node) {
  oss<<"\n";pre(node);oss<<"\n";
  Visitor::visit(node);
  post();oss<<"\n";
}
void Printer::visit(v2::CompoundStmt *node) {
  oss<<"\n";pre(node);oss<<"\n";
  Visitor::visit(node);
  post();oss<<"\n";
}
void Printer::visit(v2::BreakStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Printer::visit(v2::ContinueStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Printer::visit(v2::ReturnStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Printer::visit(v2::Expr *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Printer::visit(v2::DeclStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
void Printer::visit(v2::ExprStmt *node) {
  pre(node);
  Visitor::visit(node);
  post();
}
