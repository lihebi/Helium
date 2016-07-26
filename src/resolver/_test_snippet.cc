#include "snippet.h"
#include "utils/utils.h"
#include <gtest/gtest.h>

/**
 * Very old code, DEPRECATED
 */
TEST(snippet_test_case, DISABLED_registry_test) {
  ctags_load("test/benchmark/snippet.tag");
  SnippetRegistry::Instance()->Resolve("domain_ctx");
}

TEST(snippet_test_case, query_code_test) {
  const char* raw=R"prefix(
enum context { domain_ctx, owner_ctx, mailname_ctx, hostname_ctx };
)prefix";
  std::vector<std::string> result = query_code(raw, "//enum/block/decl/name");
  // for (std::string s : result) {
  //   std::cout <<s  << "\n";
  // }
  ASSERT_EQ(result.size(), 4);
}

/**
 * DEPRECATED
 */
TEST(snippet_test_case, DISABLED_snippet_test) {
  ctags_load("test/benchmark/snippet.tag");
  std::vector<CtagsEntry> entries = ctags_parse("domain_ctx");
  ASSERT_EQ(entries.size(), 1);
  Snippet* snippet = new Snippet(entries[0]);
  std::set<std::string> keys = snippet->GetSignatureKey();
  ASSERT_EQ(keys.size(), 5);
  // signature verify
  std::set<SnippetKind> kinds = snippet->GetSignature("domain_ctx");
  ASSERT_EQ(kinds.size(), 1);
  EXPECT_EQ(*kinds.begin(), SK_EnumMember);
  // signature verify 2
  kinds = snippet->GetSignature("context");
  ASSERT_EQ(kinds.size(), 1);
  EXPECT_NE(kinds.find(SK_Enum), kinds.end());
}
