#include "resolver.h"
#include "utils/utils.h"
#include "workflow/helium_utils.h"

#include <gtest/gtest.h>

// TEST(resolver_test_case, io_resolver_test) {
//   std::string helium_home = load_helium_home();
//   SystemResolver::Instance()->Load(helium_home + "/systype.tags");
//   Doc doc;
//   const char* raw = R"prefix(
// 	u_char *eom, *cp, *cp1, *rdatap;
// 	u_int class, type, dlen;
// 	int n,len=0;
// 	u_int32_t ttl;
// 	u_char data[MAXDATA*2];   /* sizeof data = 2 (2 * 1025 + 5*4) = 4140 */
// 	HEADER *hp = (HEADER *)msg;

// if (!ns_nameok((char *)data, class, NULL)) {
//             printf("Name not ok!\n");
//             hp->rcode = 1;
//             return (-1);
//           }
// )prefix";
//   utils::string2xml(raw, doc);
//   NodeList if_nodes = find_nodes(doc, NK_If);
//   ASSERT_EQ(if_nodes.size(), 1);
//   VariableList result;
//   resolver::get_undefined_vars(if_nodes[0], result);
//   // for (Variable var : result) {
//   //   std::cout <<var.Name()  << ":";
//   //   std::cout <<var.GetType().ToString()  << "\n";
//   // }
//   ASSERT_EQ(result.size(), 3);
//   // EXPECT_EQ(look_up(result, "data").GetType().ToString(), "u_char");
//   EXPECT_EQ(look_up(result, "class").GetType().ToString(), "u_int");
//   EXPECT_EQ(look_up(result, "hp").GetType().ToString(), "HEADER*");
  
// }

// TEST(resolver_test_case, for_io) {
//   Doc doc;
//   const char* raw = R"prefix(
// int hash_bulk_move = 3;
// static bool expanding = false;

// void foo() {
// int ii = 0;
// for (ii = 0; ii < hash_bulk_move && expanding; ++ii) {
// }
// }

// )prefix";
//   utils::string2xml(raw, doc);
//   NodeList for_nodes = find_nodes(doc, NK_For);
//   ASSERT_EQ(for_nodes.size(), 1);
//   VariableList result;
//   resolver::get_undefined_vars(for_nodes[0], result);
//   ASSERT_EQ(result.size(), 3);
// }

// TEST(resolver_test_case, switch_io) {
//   Doc doc;
//   const char* raw = R"prefix(

// int a,b;
// char c,d;
// switch(a) {
// case 1: {
// b=c;
// }
// case 2: {
// d = c;
// }
// }

// )prefix";
//   utils::string2xml(raw, doc);
//   NodeList switch_nodes = find_nodes(doc, NK_Switch);
//   ASSERT_EQ(switch_nodes.size(), 1);
//   VariableList result;
//   Node expr = switch_get_condition_expr(switch_nodes[0]);
//   // expr.print(std::cout);
//   // for (auto n : expr.select_nodes("..//expr/name")) {
//   //   std::cout << get_text(n.node()) << '\n';
//   // }
//   std::set<std::string> ids = ast::get_var_ids(expr);
//   ASSERT_EQ(ids.size(), 1);
//   resolver::get_undefined_vars(switch_nodes[0], result);
//   ASSERT_EQ(result.size(), 4);
// }

// TEST(resolver_test_case, system) {
//   SystemResolver::Instance()->Load("systype.tags");
//   ASSERT_TRUE(SystemResolver::Instance()->Has("uint8_t"));
//   std::string output = SystemResolver::Instance()->ResolveType("uint8_t");
//   EXPECT_EQ(output, "unsigned char");
//   output = SystemResolver::Instance()->ResolveType("u_int");
//   EXPECT_EQ(output, "unsigned int");
//   output = SystemResolver::Instance()->ResolveType("u_long");
//   // EXPECT_EQ(output, "unsigned long int");
//   // on Darwin, it is unsigned long
//   // on GNU Linux, it is unsigned long int
//   // I may need to unify this
//   EXPECT_TRUE(output == "unsigned long int" || output == "unsigned long");
//   output = SystemResolver::Instance()->ResolveType("u_char");
//   EXPECT_EQ(output, "unsigned char");
//   output = SystemResolver::Instance()->ResolveType("u_int8_t");
//   EXPECT_EQ(output, "unsigned char");
//   output = SystemResolver::Instance()->ResolveType("u_int32_t");
//   EXPECT_EQ(output, "unsigned int");
// }
