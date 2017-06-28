#include "helium/parser/AST.h"
#include <iostream>

#include "helium/parser/SourceManager.h"

#include "helium/utils/StringUtils.h"


std::set<std::string> TokenNode::getIdToResolve() {
  return utils::extract_id_to_resolve(Text);
}

std::set<std::string> DeclStmt::getIdToResolve() {
  return utils::extract_id_to_resolve(Text);
}

std::set<std::string> ExprStmt::getIdToResolve() {
  return utils::extract_id_to_resolve(Text);
}

std::set<std::string> Expr::getIdToResolve() {
  return utils::extract_id_to_resolve(Text);
}
