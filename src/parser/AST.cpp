#include "helium/parser/AST.h"
#include <iostream>

#include "helium/parser/SourceManager.h"


using namespace v2;

std::set<std::string> v2::TokenNode::getIdToResolve() {
  return extract_id_to_resolve(Text);
}

std::set<std::string> v2::DeclStmt::getIdToResolve() {
  return extract_id_to_resolve(Text);
}

std::set<std::string> v2::ExprStmt::getIdToResolve() {
  return extract_id_to_resolve(Text);
}

std::set<std::string> v2::Expr::getIdToResolve() {
  return extract_id_to_resolve(Text);
}
