#include <gtest/gtest.h>
#include "resolver/IOResolver.hpp"

void simplify_variable_name(std::string& s);

TEST(first_case, first) {
  EXPECT_EQ(1,1);
}

TEST(second_case, second) {
  std::string s = "a->b";
  simplify_variable_name(s);
  EXPECT_EQ(s, "a");

  s = "abcd[54].xyz";
  simplify_variable_name(s);
  EXPECT_EQ(s, "abcd");

  s = "abcd_efg.ass";
  simplify_variable_name(s);
  EXPECT_EQ(s, "abcd_efg");
}
