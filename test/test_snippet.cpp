#include <gtest/gtest.h>

#include "helium/resolver/snippet.h"
#include "helium/resolver/snippet_db.h"
#include "helium/utils/fs_utils.h"

#include "helium/resolver/cache.h"
#include "helium/resolver/SnippetAction.h"
#include "helium/resolver/SnippetV2.h"


#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;

using namespace std;



const char *snippet_a_c = R"prefix(
int global_a;
struct TempA {
} temp_a;

int aoo() {
  struct A1 a;
  struct B b;
  foo();
}
)prefix";

const char *snippet_a_h = R"prefix(
struct A1 {
};
struct A2 {
  struct A1;
};

typedef struct A2 A2;
typedef struct A3 {
  struct A2 a2;
} A3Alias;
)prefix";

const char *snippet_b_c = R"prefix(
void foo() {
  bar();
  return;
}
void bar() {
  return;
}
)prefix";

const char *snippet_b_h = R"prefix(
void foo();
void bar();
struct B {
  int b;
};
)prefix";

const char *snippet_sub_c = R"prefix(
void subc() {
  foo();
  bar();
}
)prefix";

const char *snippet_lib_c = R"prefix(
void libc() {
  foo();
}
)prefix";

class SnippetTest : public ::testing::Test {
public:
  virtual void SetUp() {
    // create folder
    fs::path temp_dir = fs::temp_directory_path();
    unique_dir = fs::unique_path(temp_dir / "%%%%-%%%%-%%%%-%%%%");
    fs::create_directories(unique_dir);
    fs::create_directories(unique_dir / "src");
    fs::create_directories(unique_dir / "include");
    fs::create_directories(unique_dir / "lib");
    fs::create_directories(unique_dir / "src" / "sub");

    fs::path file_a_c = unique_dir / "src" / "a.c";
    fs::path file_a_h = unique_dir / "src" / "a.h";
    fs::path file_b_c = unique_dir / "src" / "b.c";
    fs::path file_b_h = unique_dir / "include" / "b.h";
    fs::path file_sub_c = unique_dir / "src" / "sub" / "sub.c";
    fs::path file_lib_c = unique_dir / "lib" / "lib.c";

    // write source file
    fs::ofstream of;
    utils::write_file(file_a_c.string(), snippet_a_c);
    utils::write_file(file_a_h.string(), snippet_a_h);
    utils::write_file(file_b_c.string(), snippet_b_c);
    utils::write_file(file_b_h.string(), snippet_b_h);
    utils::write_file(file_sub_c.string(), snippet_sub_c);
    utils::write_file(file_lib_c.string(), snippet_lib_c);
     

    // Create cache
    std::string target_dir_name = fs::canonical(unique_dir).string();
    std::replace(target_dir_name.begin(), target_dir_name.end(), '/', '_');
    fs::path user_home(getenv("HOME"));
    fs::path helium_home = user_home / ".helium.d";
    if (!fs::exists(helium_home)) {
      fs::create_directory(helium_home);
    }
    fs::path cache_dir = helium_home / "cache";
    target_cache_dir = fs::path(helium_home / "cache" / target_dir_name);
    // 1. preprocess
    fs::create_directories(target_cache_dir);
    create_src(unique_dir, target_cache_dir);
    create_cpp(target_cache_dir);
    // 2. create tagfile
    create_tagfile(target_cache_dir);
    // 3. create clang-snippet
    create_clang_snippet(target_cache_dir);
    // 4. create snippet db
    create_snippet_db(target_cache_dir);
    utils::write_file((target_cache_dir/"valid").string(), "");
    
    std::cout << "Created " << unique_dir.string() << "\n";

    // Check if entries are in database
    SnippetDB::Instance()->Load(target_cache_dir / "snippet.db", target_cache_dir / "code");

    {
      set<int> ids = SnippetDB::Instance()->LookUp("A");
      ASSERT_EQ(ids.size(), 1);
      int id = *ids.begin();
      SnippetMeta meta = SnippetDB::Instance()->GetMeta(id);
      std::cout << "===== A:" << "\n";
      meta.dump(std::cout);
      std::cout << "===== Deps:" << "\n";
      std::string code = SnippetDB::Instance()->GetCode(id);
      set<int> deps = SnippetDB::Instance()->GetDep(id);
      for (int dep : deps) {
        SnippetMeta dep_meta = SnippetDB::Instance()->GetMeta(dep);
        dep_meta.dump(std::cout);
      }
      std::cout << "Code: " << code << "\n";
    }
  } 
  virtual void TearDown() {
    // TODO remove from cache
    // fs::remove_all(target_cache_dir);
    // remove it
    // fs::remove_all(unique_dir);
  }
private:
  fs::path unique_dir;
  fs::path target_cache_dir;
};

// TEST_F(SnippetTest, MySnippetTest) {
// }



class NewSnippetTest : public ::testing::Test {
protected:
  void SetUp() {
    fs::path user_home = getenv("HOME");
    fs::path dir = user_home / "github" / "benchmark" / "craft" / "snippet";
    file_a_c = dir / "src" / "a.c";
    file_a_h = dir / "src" / "a.h";
    file_var_c = dir / "src" / "var.c";
    file_b_c = dir / "src" / "b.c";
    file_b_h = dir / "include" / "b.h";
    file_sub_c = dir / "src" / "sub" / "sub.c";
    file_lib_c = dir / "lib" / "lib.c";
  }
  void TearDown() {}

  fs::path file_a_c;
  fs::path file_a_h;
  fs::path file_b_c;
  fs::path file_b_h;
  fs::path file_sub_c;
  fs::path file_lib_c;
  fs::path file_var_c;
};



TEST_F(NewSnippetTest, MyTest) {
  {
    // std::cout << file_a_h.string() << "\n";
    std::vector<v2::Snippet*> snippets = createSnippets(file_a_h);
    v2::SnippetManager *manager = new v2::SnippetManager();
    manager->add(snippets);
    manager->process();
    // manager->dump(std::cout);
    v2::Snippet *A1 = manager->get("A1", "RecordSnippet");
    v2::Snippet *A2 = manager->get("A2", "RecordSnippet");
    v2::Snippet *A3 = manager->get("A3", "RecordSnippet");
    v2::Snippet *A3Alias = manager->get("A3Alias", "TypedefSnippet");
    ASSERT_TRUE(A1);
    ASSERT_TRUE(A2);
    ASSERT_TRUE(A3);
    ASSERT_TRUE(A3Alias);

    EXPECT_EQ(A1->getCode(), "struct A1 {\n}");
    EXPECT_EQ(A2->getCode(), "struct A2 {\n  struct A1;\n}");
    EXPECT_EQ(A3->getCode(), "struct A3 {\n  struct A2 a2;\n}");
  }
  {
    std::vector<v2::Snippet*> snippets = createSnippets(file_a_c);
    v2::SnippetManager *manager = new v2::SnippetManager();
    manager->add(snippets);
    manager->process();

    v2::Snippet *global_a = manager->get("global_a", "VarSnippet");
    v2::Snippet *temp_a = manager->get("temp_a", "VarSnippet");
    v2::Snippet *myvar = manager->get("myvar", "VarSnippet");
    v2::Snippet *aoo = manager->get("aoo", "FunctionSnippet");
    ASSERT_TRUE(global_a);
    ASSERT_TRUE(temp_a);
    ASSERT_TRUE(myvar);
    ASSERT_TRUE(aoo);

    EXPECT_EQ(global_a->getCode(), "int global_a");
    // FIXME
    EXPECT_EQ(temp_a->getCode(), "struct TempA {\n} temp_a");
    EXPECT_EQ(myvar->getCode(), "struct ABC {\n  int a;\n} myvar");
    EXPECT_EQ(aoo->getCode(), "int aoo() {\n  struct A1 a;\n  struct B b;\n  foo();\n}");
  }
  {
    std::vector<v2::Snippet*> snippets = createSnippets(file_var_c);
    v2::SnippetManager *manager = new v2::SnippetManager();
    manager->add(snippets);
    manager->process();

    v2::Snippet *a = manager->get("a", "VarSnippet");
    v2::Snippet *b = manager->get("b", "VarSnippet");
    v2::Snippet *c = manager->get("c", "VarSnippet");
    v2::Snippet *d = manager->get("d", "VarSnippet");
    v2::Snippet *e = manager->get("e", "VarSnippet");
    v2::Snippet *f = manager->get("f", "VarSnippet");
    v2::Snippet *gg = manager->get("gg", "VarSnippet");
    v2::Snippet *p = manager->get("p", "VarSnippet");
    v2::Snippet *longvar = manager->get("longvar", "VarSnippet");
    v2::Snippet *long_var = manager->get("long_var", "VarSnippet");
    v2::Snippet *longvarinit = manager->get("longvarinit", "VarSnippet");
    
    ASSERT_TRUE(a && b && c && d && e && f && gg && p && longvar && long_var && longvarinit);

    EXPECT_EQ(a->getCode(), "int a=0");
    EXPECT_EQ(b->getCode(), "int b,c");
    EXPECT_EQ(c->getCode(), "int b,c");
    EXPECT_EQ(d->getCode(), "int d=0,e");
    EXPECT_EQ(e->getCode(), "int d=0,e");
    EXPECT_EQ(f->getCode(), "int f,gg=0");
    EXPECT_EQ(gg->getCode(), "int f,gg=0");
    EXPECT_EQ(p->getCode(), "int *p");
    EXPECT_EQ(longvar->getCode(), "int longvar");
    EXPECT_EQ(long_var->getCode(), "int long_var");
    EXPECT_EQ(longvarinit->getCode(), "int longvarinit=88");
  }

  {
    // test dependency
    v2::SnippetManager *manager = new v2::SnippetManager();
    manager->add(createSnippets(file_a_c));
    manager->add(createSnippets(file_a_h));
    manager->add(createSnippets(file_b_c));
    manager->add(createSnippets(file_b_h));
    manager->add(createSnippets(file_sub_c));
    manager->add(createSnippets(file_lib_c));
    manager->process();
  }

  {
    // Test save and load back
    std::vector<v2::Snippet*> snippets = createSnippets(file_var_c);
    v2::SnippetManager *manager = new v2::SnippetManager();
    manager->add(snippets);
    manager->process();

    fs::path to_file = fs::unique_path(fs::temp_directory_path() / "%%%%-%%%%-%%%%-%%%%");
    manager->saveSnippet(to_file);
    v2::SnippetManager *manager2 = new v2::SnippetManager();
    manager2->loadSnippet(to_file);

    // EXPECT_TRUE(manager2->equivalent(manager));

    std::vector<v2::Snippet*> snippets2 = manager2->getSnippets();
    // check snippet and snippet2
    ASSERT_EQ(snippets.size(), snippets2.size());
    for (int i=0;i<snippets.size();i++) {
      v2::Snippet *s1 = snippets[i];
      v2::Snippet *s2 = snippets2[i];
      EXPECT_EQ(s1->getName(), s2->getName());
    }
    
  }
}
