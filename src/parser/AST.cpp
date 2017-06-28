#include "helium/parser/AST.h"
#include <iostream>

#include "helium/parser/SourceManager.h"




std::set<std::string> TokenNode::getIdToResolve() {
  return extract_id_to_resolve(Text);
}

std::set<std::string> DeclStmt::getIdToResolve() {
  return extract_id_to_resolve(Text);
}

std::set<std::string> ExprStmt::getIdToResolve() {
  return extract_id_to_resolve(Text);
}

std::set<std::string> Expr::getIdToResolve() {
  return extract_id_to_resolve(Text);
}
