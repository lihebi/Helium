#include "ast.h"
#include "utils.h"
#include <gtest/gtest.h>

using namespace ast;

/*******************************
 ** AST sub classes
 *******************************/

TEST(ast_test_case, decl_test) {
  Doc doc;
  const char *raw = R"prefix(
int a;
int a=0;
int a,b;
int a=0,b;
int a,b=0;
int a=0,b=1;
int a=0,b,c=3;

)prefix";
  utils::string2xml(raw, doc);
  NodeList nodes = find_nodes(doc, NK_DeclStmt);
  ASSERT_EQ(nodes.size(), 7);
  NodeList decls;
  decls = decl_stmt_get_decls(nodes[0]);
  ASSERT_EQ(decls.size(), 1);
  EXPECT_EQ(decl_get_name(decls[0]), "a");
  EXPECT_EQ(decl_get_type(decls[0]), "int");
}


TEST(ast_test_case, function_test) {
  Doc doc;
  const char *raw = R"prefix(

int myfunc(int a, int b) {
  int b;
}

)prefix";
  utils::string2xml(raw, doc);
  NodeList nodes = find_nodes(doc, NK_Function);
  EXPECT_EQ(nodes.size(), 1);
  Node myfunc = nodes[0];
  EXPECT_EQ(function_get_return_type(myfunc), "int");
  EXPECT_EQ(function_get_name(myfunc), "myfunc");
  NodeList params = function_get_params(myfunc);
  ASSERT_EQ(params.size(), 2);
  EXPECT_EQ(param_get_type(params[0]), "int");
  EXPECT_EQ(param_get_name(params[0]), "a");
  EXPECT_EQ(param_get_type(params[1]), "int");
  EXPECT_EQ(param_get_name(params[1]), "b");
}

TEST(ast_test_case, for_test) {
  Doc doc;
  const char *raw = R"prefix(

for (int i=0,c=2;i<8;++i) {
}

)prefix";
  utils::string2xml(raw, doc);
  NodeList nodes = find_nodes(doc, NK_For);
  ASSERT_EQ(nodes.size(), 1);
  Node myfor = nodes[0];
  NodeList decls = for_get_init_decls(myfor);
  ASSERT_EQ(decls.size(), 2);
  EXPECT_EQ(decl_get_name(decls[0]), "i");
  EXPECT_EQ(decl_get_type(decls[0]), "int");
  EXPECT_EQ(decl_get_name(decls[1]), "c");
  EXPECT_EQ(decl_get_type(decls[1]), "int");
  
  // std::map<std::string, std::string> vars = for_get_init_detail(myfor);
  // ASSERT_EQ(vars.size(), 2);
  // EXPECT_EQ(vars["i"], "int");
  // EXPECT_EQ(vars["c"], "int");
  // Node condition_expr = for_get_condition_expr(myfor);
  // Node incr_expr = for_get_incr_expr(myfor);
  // Node block = for_get_block(myfor);
}

/*******************************
 ** helper functions
 *******************************/

TEST(ast_test_case, find_nodes_test) {
  Doc doc;
  const char* raw = R"prefix(
int func() {
int a;
int b;
memcpy(a,b);

// this is comment
  strcpy(a,b);
}
)prefix";

  // remove leading spaces
  while (isspace(*raw)) raw++;

  utils::string2xml(raw, doc);
  NodeList nodes = find_nodes(doc, NK_Function);
  ASSERT_EQ(nodes.size(), 1);
  EXPECT_EQ(kind(nodes[0]), NK_Function);
  nodes = find_nodes(doc, NK_ExprStmt);
  ASSERT_EQ(nodes.size(), 2);
  Node node;
  node = find_node_on_line(doc, NK_ExprStmt, 4);
  ASSERT_TRUE(node);

  std::vector<NodeKind> kinds;
  kinds.push_back(NK_DeclStmt);
  kinds.push_back(NK_ExprStmt);

  std::vector<int> lines  {4,7};
  nodes = find_nodes_on_lines(doc, kinds, lines);
  ASSERT_EQ(nodes.size(), 2);
  // std::cout <<get_text(nodes[0])  << "\n";
  // std::cout <<get_text(nodes[1])  << "\n";
}
