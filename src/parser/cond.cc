#include "cond.h"
#include <gtest/gtest.h>

TEST(CondCase, CreateTest) {
  Cond *cond = CondFactory::Create("a>b");
}


OpKind Str2OpKind(std::string text) {
  if (text == ">") return OK_Greater;
  if (text == "<") return OK_Less;
  if (text == "=") return OK_Equal;
  if (text == "!=") return OK_NotEqual;
  if (text == ">=") return OK_GreaterOrEqual;
  if (text == "<=") return OK_LessOrEqual;
  throw HeliumException("Str2OpKind");
}

std::string OpKind2Str(OpKind op) {
  switch(op) {
  case OK_Greater: return ">";
  case OK_Less: return "<";
  case OK_Equal: return "=";
  case OK_NotEqual: return "!=";
  case OK_GreaterOrEqual: return ">=";
  case OK_LessOrEqual: return "<=";
  default: throw HeliumException("OpKind2Str");
  }
}
