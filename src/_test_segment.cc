#include "segment.h"
#include "config.h"
#include "ast.h"

#include <gtest/gtest.h>

using namespace ast;

TEST(segment_test_case, context_test) {
  Config::Instance()->ParseFile("./helium.conf");
  Config::Instance()->Set("linear-search-value", "15");

  ast::Doc doc;
  const char* raw = R"prefix(

int func() {
  int a;
  int b;
  switch (a) {
  case 3: {
    a++;
    b++;
    break;
  }
  case 8: {
    b++;
    if (a>b) {
// @Helium
      a = a+b;
    }
    break;
  }
  }
}

)prefix";
  utils::string2xml(raw, doc);
  // std::vector<NodeKind> kinds;
  // kinds.push_back(NK_DeclStmt);
  // kinds.push_back(NK_ExprStmt);
  Node comment_node = ast::find_node_containing_str(doc, NK_Comment, "@Helium");
  ASSERT_TRUE(comment_node);
  Node node = next_sibling(comment_node);
  EXPECT_EQ(ast::get_text(node), "a = a+b;");
  Segment seg;
  ASSERT_FALSE(seg.IsValid());
  seg.PushBack(node);

  ASSERT_TRUE(seg.IsValid());
  // for (;seg.IsValid();) {
  //   seg.IncreaseContext();
  //   std::cout <<"========"  << "\n";
  //   std::cout <<seg.GetText()  << "\n";
  // }
}

TEST(segment_test_case, io_variable_test) {
  ast::Doc doc;
  const char* raw = R"prefix(

int func() {
  int a;
  int b;
int c;
  switch (a) {
  case 3: {
    a++;
    b++;
    break;
  }
  case 8: {
    b++;
    if (a>b) {
// @Helium
      b += c;
      a = a+b;
    }
    break;
  }
  }
}

)prefix";
  utils::string2xml(raw, doc);
  Node comment_node = ast::find_node_containing_str(doc, NK_Comment, "@Helium");
  Node n = next_sibling(comment_node);
  Segment seg;
  seg.PushBack(n);
  seg.PushBack(next_sibling(n));
  ASSERT_TRUE(seg.IsValid());

  // resolving vars
  seg.ResolveInput();
  VariableList vars = seg.GetInputVariables();
  ASSERT_EQ(vars.size(), 3);
  Variable a = look_up(vars, "a");
  Variable b = look_up(vars, "b");
  Variable c = look_up(vars, "c");
  ASSERT_TRUE(a);
  ASSERT_TRUE(b);
  ASSERT_TRUE(c);
  EXPECT_EQ(a.GetType().ToString(), "int");
  EXPECT_EQ(b.GetType().ToString(), "int");
  EXPECT_EQ(c.GetType().ToString(), "int");
}
