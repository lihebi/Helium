#include "type.h"
#include "utils.h"
#include <gtest/gtest.h>

using namespace ast;
TEST(type_test_case, var_from_node_test) {
  Doc doc;
  const char* raw = R"prefix(

int a = a+b;
u_char *eom, *cp, *cp1, *rdatap;

)prefix";
  utils::string2xml(raw, doc);
  NodeList nodes = find_nodes(doc, NK_DeclStmt);
  ASSERT_EQ(nodes.size(), 2);
  VariableList vars;
  // int a
  vars = var_from_node(nodes[0]);
  ASSERT_EQ(vars.size(), 1);
  EXPECT_EQ(vars[0].Name(), "a");
  EXPECT_EQ(vars[0].GetType().ToString(), "int");
  // uchar *
  vars = var_from_node(nodes[1]);
  ASSERT_EQ(vars.size(), 4);
  EXPECT_EQ(vars[0].Name(), "eom");
  EXPECT_EQ(vars[0].GetType().ToString(), "u_char*");
  EXPECT_EQ(vars[1].Name(), "cp");

}
