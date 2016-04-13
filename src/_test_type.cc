#include "type.h"
#include "utils.h"
#include <gtest/gtest.h>
#include "resolver.h"
using namespace ast;
TEST(type_test_case, var_from_node_test) {
  SystemResolver::Instance()->Load("systype.tags");
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


/*******************************
 ** test input code
 *******************************/

inline void func(std::string type, std::string name, std::string expect) {
  Variable v(type, name);
  std::string code = get_input_code(v);
  utils::trim(expect);
  EXPECT_EQ(code, expect);
}

TEST(type_test_case, DISABLED_int_input_code) {
  SystemResolver::Instance()->Load("systype.tags");
  std::string type = "int";
  std::string name = "myvar";
  const char* s =        R"prefix(

int myvar;
scanf("%d", &myvar);

)prefix";
  
  Variable v(type, name);
  std::string code = get_input_code(v);
  std::string expect = s;
  utils::trim(expect);
  EXPECT_EQ(code, expect);
}

TEST(type_test_case, DISABLED_char_input_code) {
  std::string type = "char";
  std::string name = "myvar";
  const char* s =        R"prefix(

char myvar;
scanf("%c", &myvar);

)prefix";
  
  Variable v(type, name);
  std::string code = get_input_code(v);
  std::string expect = s;
  utils::trim(expect);
  EXPECT_EQ(code, expect);
}

// TEST(type_test_case, malloc_0) {
//   char *s = (char*)malloc(0);
//   EXPECT_TRUE(s == NULL);
//   EXPECT_FALSE(s);
// }

TEST(type_test_case, DISABLED_char_star_input_code) {
  std::string type = "char*";
  std::string name = "myvar";
  const char* s =        R"prefix(

scanf("%d", &helium_size);
char* myvar;
if (helium_size == 0) {
  myvar = NULL;
} else {
  myvar = (char*)malloc(sizeof(char)*helium_size);
  scanf("%s", myvar);
}

)prefix";
  
  Variable v(type, name);
  std::string code = get_input_code(v);
  std::string expect = s;
  utils::trim(expect);
  EXPECT_EQ(code, expect);
}

TEST(type_test_case, DISABLED_uchar_star_input_code) {
  SystemResolver::Instance()->Load("systype.tags");
  std::string type = "u_char*";
  std::string name = "myvar";
  const char* s =        R"prefix(

scanf("%d", &helium_size);
unsigned char* myvar;
if (helium_size == 0) {
  myvar = NULL;
} else {
  myvar = (unsigned char*)malloc(sizeof(unsigned char)*helium_size);
  scanf("%s", myvar);
}

)prefix";
  
  Variable v(type, name);
  EXPECT_EQ(v.GetType().Kind(), TK_Primitive);
  std::string code = get_input_code(v);
  std::string expect = s;
  utils::trim(expect);
  EXPECT_EQ(code, expect);
}

