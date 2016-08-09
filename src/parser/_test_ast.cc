#include "parser/xmlnode.h"
#include "parser/xmlnode_helper.h"
#include "utils/utils.h"
#include <gtest/gtest.h>

/*******************************
 ** AST sub classes
 *******************************/

TEST(ast_test_case, decl_test) {
  XMLDoc doc;
  const char *raw = R"prefix(
int a;
int a=0;
int a,b;
int a=0,b;
int a,b=0;
int a=0,b=1;
int a=0,b,c=3;
// 8
u_char *eom, *cp, *cp1, *rdatap;
// 9
int a[3][8];
)prefix";
  utils::string2xml(raw, doc);
  XMLNodeList nodes = find_nodes(doc, NK_DeclStmt);
  ASSERT_EQ(nodes.size(), 9);
  XMLNodeList decls;
  // 1
  decls = decl_stmt_get_decls(nodes[0]);
  ASSERT_EQ(decls.size(), 1);
  EXPECT_EQ(decl_get_name(decls[0]), "a");
  EXPECT_EQ(decl_get_type(decls[0]), "int");
  // a=0
  decls = decl_stmt_get_decls(nodes[1]);
  ASSERT_EQ(decls.size(), 1);
  EXPECT_EQ(decl_get_name(decls[0]), "a");
  EXPECT_EQ(decl_get_type(decls[0]), "int");
  // a,b
  decls = decl_stmt_get_decls(nodes[2]);
  ASSERT_EQ(decls.size(), 2);
  EXPECT_EQ(decl_get_name(decls[0]), "a");
  EXPECT_EQ(decl_get_type(decls[0]), "int");
  EXPECT_EQ(decl_get_name(decls[1]), "b");
  EXPECT_EQ(decl_get_type(decls[1]), "int");
  // 8 u_char
  decls = decl_stmt_get_decls(nodes[7]);
  ASSERT_EQ(decls.size(), 4);
  EXPECT_EQ(decl_get_name(decls[0]), "eom");
  EXPECT_EQ(decl_get_type(decls[0]), "u_char*");
  EXPECT_EQ(decl_get_name(decls[1]), "cp");
  EXPECT_EQ(decl_get_type(decls[1]), "u_char*");
  // 9 int a[3][8]
  decls = decl_stmt_get_decls(nodes[8]);
  ASSERT_EQ(decls.size(), 1);
  EXPECT_EQ(decl_get_name(decls[0]), "a");
  // EXPECT_EQ(decl_get_type(decls[0]), "int[][]"); // TODO the constant size need to be cpatured?
  EXPECT_EQ(decl_get_type(decls[0]), "int");
}


TEST(ast_test_case, function_test) {
  XMLDoc doc;
  const char *raw = R"prefix(

int myfunc(int a, int b) {
  int b;
}

)prefix";
  utils::string2xml(raw, doc);
  XMLNodeList nodes = find_nodes(doc, NK_Function);
  EXPECT_EQ(nodes.size(), 1);
  XMLNode myfunc = nodes[0];
  EXPECT_EQ(function_get_return_type(myfunc), "int");
  EXPECT_EQ(function_get_name(myfunc), "myfunc");
  XMLNodeList params = function_get_params(myfunc);
  ASSERT_EQ(params.size(), 2);
  EXPECT_EQ(param_get_type(params[0]), "int");
  EXPECT_EQ(param_get_name(params[0]), "a");
  EXPECT_EQ(param_get_type(params[1]), "int");
  EXPECT_EQ(param_get_name(params[1]), "b");
}

TEST(ast_test_case, for_test) {
  XMLDoc doc;
  const char *raw = R"prefix(

for (int i=0,c=2;i<8;++i) {
}

)prefix";
  utils::string2xml(raw, doc);
  XMLNodeList nodes = find_nodes(doc, NK_For);
  ASSERT_EQ(nodes.size(), 1);
  XMLNode myfor = nodes[0];
  XMLNodeList decls = for_get_init_decls_or_exprs(myfor);
  ASSERT_EQ(decls.size(), 2);
  EXPECT_EQ(decl_get_name(decls[0]), "i");
  EXPECT_EQ(decl_get_type(decls[0]), "int");
  EXPECT_EQ(decl_get_name(decls[1]), "c");
  EXPECT_EQ(decl_get_type(decls[1]), "int");
  
  // std::map<std::string, std::string> vars = for_get_init_detail(myfor);
  // ASSERT_EQ(vars.size(), 2);
  // EXPECT_EQ(vars["i"], "int");
  // EXPECT_EQ(vars["c"], "int");
  XMLNode condition_expr = for_get_condition_expr(myfor);
  ASSERT_TRUE(condition_expr);
  XMLNode incr_expr = for_get_incr_expr(myfor);
  ASSERT_TRUE(incr_expr);
  // XMLNode block = for_get_block(myfor);
}

/*******************************
 ** helper functions
 *******************************/

TEST(ast_test_case, find_nodes_test) {
  XMLDoc doc;
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
  XMLNodeList nodes = find_nodes(doc, NK_Function);
  ASSERT_EQ(nodes.size(), 1);
  EXPECT_EQ(xmlnode_to_kind(nodes[0]), NK_Function);
  nodes = find_nodes(doc, NK_ExprStmt);
  ASSERT_EQ(nodes.size(), 2);
  XMLNode node;
  node = find_node_on_line(doc, NK_ExprStmt, 4);
  ASSERT_TRUE(node);

  std::vector<XMLNodeKind> kinds;
  kinds.push_back(NK_DeclStmt);
  kinds.push_back(NK_ExprStmt);

  std::vector<int> lines  {4,7};
  nodes = find_nodes_on_lines(doc, kinds, lines);
  ASSERT_EQ(nodes.size(), 2);
  // std::cout <<get_text(nodes[0])  << "\n";
  // std::cout <<get_text(nodes[1])  << "\n";
}

/**
 * This test is for get_var_ids.
 * The reason is when the expr is ~a.b->c~, it will be <expr><name><name></name></name></name>
 */
TEST(ast_test_case, get_var_ids) {
  XMLDoc doc;
  const char* raw = R"prefix(
for (;;) {
stats.hash_bytes =0;
stats.hash_is_expanding = 0;
}
for (ii = 0; ii < search->nkey; ++ii) {
}
)prefix";
  utils::string2xml(raw, doc);
  XMLNodeList fors = find_nodes(doc, NK_For);
  ASSERT_EQ(fors.size(), 2);
  // first for
  std::set<std::string> ids = get_var_ids(fors[0]);
  ASSERT_EQ(ids.size(), 1);
  EXPECT_EQ(ids.count("stats"), 1);
  // second for
  ids = get_var_ids(fors[1]);
  ASSERT_EQ(ids.size(), 2);
  ASSERT_EQ(ids.count("ii"), 1);
  ASSERT_EQ(ids.count("search"), 1);
}

/**
 * Test if the id to resolve from expr_stmt is correct.
 */
TEST(ast_test_case, expr_stmt_test) {
  XMLDoc doc;
  const char* raw = R"prefix(

a = a+b;
memcpy(a,b);
memcpy(cp1, cp, dlen - n);
cp1 = data + strlen((char *)data) + 1;

)prefix";
  utils::string2xml(raw, doc);
  XMLNodeList nodes = find_nodes(doc, NK_ExprStmt);
  ASSERT_EQ(nodes.size(), 4);
  // a = a+b;
  std::set<std::string> ids;
  // ids = expr_stmt_get_var_ids(nodes[0]);
  ids = get_var_ids(nodes[0]);
  ASSERT_EQ(ids.size(), 2);
  EXPECT_TRUE(ids.find("a") != ids.end());
  EXPECT_TRUE(ids.find("b") != ids.end());
  // memcpy(a,b)
  // ids = expr_stmt_get_var_ids(nodes[1]);
  ids = get_var_ids(nodes[1]);
  ASSERT_EQ(ids.size(), 2);
  EXPECT_TRUE(ids.find("a") != ids.end());
  EXPECT_TRUE(ids.find("b") != ids.end());
  // memcpy(cp1, cp)
  // ids = expr_stmt_get_var_ids(nodes[2]);
  ids = get_var_ids(nodes[2]);
  ASSERT_EQ(ids.size(), 4);
  EXPECT_TRUE(ids.find("cp1") != ids.end());
  EXPECT_TRUE(ids.find("cp") != ids.end());
  EXPECT_TRUE(ids.find("dlen") != ids.end());
  EXPECT_TRUE(ids.find("n") != ids.end());
  // cp1 =
  // ids = expr_stmt_get_var_ids(nodes[3]);
  ids = get_var_ids(nodes[3]);
  // FIXME srcml doesn't tell me if (char) is a type or an id. I need more precise information to continue.
  // ASSERT_EQ(ids.size(), 2);
  // EXPECT_TRUE(ids.find("cp1") != ids.end());
  // EXPECT_TRUE(ids.find("data") != ids.end());
}

// TEST(ast_test_case, DISABLED_decl_test_deprecated) {
//   std::string code;
//   std::map<std::string, std::string> decls;
//   // single
//   code = "int a;";
//   decls = get_decl_detail(code);
//   EXPECT_EQ(decls.size(), 1);
//   EXPECT_EQ(decls["a"], "int");
//   // without ;
//   code = "int a";
//   decls = get_decl_detail(code);
//   EXPECT_EQ(decls.size(), 1);
//   EXPECT_EQ(decls["a"], "int");
//   // double ;;
//   code = "int a;;";
//   decls = get_decl_detail(code);
//   EXPECT_EQ(decls.size(), 1);
//   EXPECT_EQ(decls["a"], "int");
//   // double
//   code = "int a;char b;";
//   decls = get_decl_detail(code);
//   EXPECT_EQ(decls.size(), 2);
//   EXPECT_EQ(decls["a"], "int");
//   EXPECT_EQ(decls["b"], "char");
//   // same type
//   code = "int a,b;";
//   decls = get_decl_detail(code);
//   EXPECT_EQ(decls.size(), 2);
//   EXPECT_EQ(decls["a"], "int");
//   EXPECT_EQ(decls["b"], "int");
// }
