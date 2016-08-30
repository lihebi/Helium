#include "ast_node.h"
#include <gtest/gtest.h>
#include "utils/utils.h"
#include "config/options.h"

static const std::map<XMLNodeKind, ASTNodeKind> nk2ank_m {
  {NK_Function, ANK_Function}
  , {NK_Block, ANK_Block}
  // statements
  , {NK_Stmt, ANK_Stmt}
  , {NK_ExprStmt, ANK_Stmt}
  , {NK_DeclStmt, ANK_Stmt}
  // Break, Continue, Return
  , {NK_Break, ANK_Stmt}
  , {NK_Continue, ANK_Stmt}
  , {NK_Return, ANK_Stmt}
  // , {NK_EmptyStmt, ANK_Stmt}
  // , {NK_Expr, ANK_Expr}
  // condition
  , {NK_If, ANK_If}
  , {NK_Else, ANK_Else}
  , {NK_ElseIf, ANK_ElseIf}
  , {NK_Then, ANK_Then}
  , {NK_Switch, ANK_Switch}
  , {NK_Case, ANK_Case}
  , {NK_Default, ANK_Default}
  // loop
  , {NK_While, ANK_While}
  , {NK_For, ANK_For}
  , {NK_Do, ANK_Do}
};

ASTNodeKind xmlnode_kind_to_astnode_kind(XMLNodeKind kind) {
  ASTNodeKind ret;
  if (nk2ank_m.count(kind) == 1) {
    ret = nk2ank_m.at(kind);
  } else {
    ret = ANK_Other;
  }
  return ret;
}





std::string ASTNode::POIOutputCode() {
  std::string ret;
  std::vector<Variable> vars = m_ast->GetRequiredOutputVariables(this);
  if (vars.size() > 0) {
    ret += "printf(\"HELIUM_POI = true\\n\");\n";
    ret += "fflush(stdout);\n";
    for (Variable var : vars) {
      ret += var.GetType()->GetOutputCode(var.GetName());
    }
    ret += "printf(\"HELIUM_POI_OUT_END = true\\n\");\n";
    ret += "fflush(stdout);\n";
    ret += "// @HeliumSegmentBegin\n";
  }
  return ret;
}

/**
 * Code to instrument after the POI node
 */
std::string ASTNode::POIAfterCode() {
  std::string ret;
  std::vector<Variable> vars = m_ast->GetRequiredOutputVariables(this);
  if (vars.size() > 0) {
    ret += "// @HeliumSegmentEnd\n";
  }
  return ret;
}


/**
 * Code to instument output statement after free() function call
 */
std::string ASTNode::FreedListCode() {
  std::string ret;
  std::vector<std::string> freed_exprs = m_ast->GetFreedExprs(this);
  for (std::string expr : freed_exprs) {
    ret += "printf(\"freedlist: %p\\n\", (void*)" + expr + ");\n";
  }
  return ret;
}


ASTNode* ASTNodeFactory::CreateASTNode(XMLNode xml_node, ASTNode* parent, AST *ast) {
  XMLNodeKind nk = xmlnode_to_kind(xml_node);
  ASTNode *ret = NULL;
  ASTNodeKind ank;
  ank = xmlnode_kind_to_astnode_kind(nk);
  switch (ank) {
  case ANK_Function: {
    ret = new Function(xml_node, parent, ast);
    break;
  }
  case ANK_Block: {
    ret = new Block(xml_node, parent, ast);
    break;
  }
  case ANK_Stmt: {
    ret = new Stmt(xml_node, parent, ast);
    break;
  }
  // case ANK_Expr: {
  //   ret = new Expr(xml_node, parent, ast);
  //   break;
  // }
  case ANK_If: {
    ret = new If(xml_node, parent, ast);
    break;
  }
  case ANK_Then: {
    ret = new Then(xml_node, parent, ast);
    break;
  }
  case ANK_Else: {
    ret = new Else(xml_node, parent, ast);
    break;
  }
  case ANK_ElseIf: {
    ret = new ElseIf(xml_node, parent, ast);
    break;
  }
  case ANK_Switch: {
    ret = new Switch(xml_node, parent, ast);
    break;
  }
  case ANK_Case: {
    ret = new Case(xml_node, parent, ast);
    break;
  }
  case ANK_Default: {
    ret = new Default(xml_node, parent, ast);
    break;
  }
  case ANK_While: {
    ret = new While(xml_node, parent, ast);
    break;
  }
  case ANK_For: {
    ret = new For(xml_node, parent, ast);
    break;
  }
  case ANK_Do: {
    ret = new Do(xml_node, parent, ast);
    break;
  }
  case ANK_Other: {
    // ret = new ASTOther(xml_node);
    // TODO for other xml node(like <unit>),
    // we need to smartly get the valid chlidren of that nodes,
    // and have edges from this to those nodes.
    // But currently, just NULL.
    ret = NULL;
    break;
  }
  default: {
    assert(false);
  }
  }
  return ret;
}
