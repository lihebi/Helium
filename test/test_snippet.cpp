#include <gtest/gtest.h>

#include "helium/utils/FSUtils.h"

#include "helium/type/Cache.h"
#include "helium/type/SnippetAction.h"
#include "helium/type/Snippet.h"


#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;

using namespace std;

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
    file_decl_h = dir / "src" / "decl.h";
  }
  void TearDown() {}

  fs::path file_a_c;
  fs::path file_a_h;
  fs::path file_b_c;
  fs::path file_b_h;
  fs::path file_sub_c;
  fs::path file_lib_c;
  fs::path file_var_c;
  fs::path file_decl_h;
};


TEST_F(NewSnippetTest, MyTest) {
  {
    // std::cout << file_a_h.string() << "\n";
    std::vector<Snippet*> snippets = createSnippets(file_a_h);
    SnippetManager *manager = new SnippetManager();
    manager->add(snippets);
    manager->process();
    // manager->dump(std::cout);
    Snippet *A1 = manager->getone("A1", "RecordSnippet");
    Snippet *A2 = manager->getone("A2", "RecordSnippet");
    Snippet *A2Typedef = manager->getone("A2", "TypedefSnippet");
    Snippet *A3 = manager->getone("A3", "RecordSnippet");
    Snippet *A3Alias = manager->getone("A3Alias", "TypedefSnippet");
    ASSERT_TRUE(A1);
    ASSERT_TRUE(A2);
    ASSERT_TRUE(A3);
    ASSERT_TRUE(A3Alias);

    EXPECT_EQ(A1->getCode(), "struct A1 {\n}");
    EXPECT_EQ(A2->getCode(), "struct A2 {\n  struct A1 a;\n}");
    EXPECT_EQ(A2Typedef->getCode(), "typedef struct A2 A2");
    EXPECT_EQ(A3->getCode(), "struct A3 {\n  struct A2 a2;\n}");
    EXPECT_EQ(A3Alias->getCode(), "typedef struct A3 {\n  struct A2 a2;\n} A3Alias");
  }
  {
    std::vector<Snippet*> snippets = createSnippets(file_a_c);
    SnippetManager *manager = new SnippetManager();
    manager->add(snippets);
    manager->process();

    Snippet *global_a = manager->getone("global_a", "VarSnippet");
    Snippet *temp_a = manager->getone("temp_a", "VarSnippet");
    Snippet *myvar = manager->getone("myvar", "VarSnippet");
    Snippet *aoo = manager->getone("aoo", "FunctionSnippet");
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
    std::vector<Snippet*> snippets = createSnippets(file_var_c);
    SnippetManager *manager = new SnippetManager();
    manager->add(snippets);
    manager->process();

    Snippet *a = manager->getone("a", "VarSnippet");
    Snippet *b = manager->getone("b", "VarSnippet");
    Snippet *c = manager->getone("c", "VarSnippet");
    Snippet *d = manager->getone("d", "VarSnippet");
    Snippet *e = manager->getone("e", "VarSnippet");
    Snippet *f = manager->getone("f", "VarSnippet");
    Snippet *gg = manager->getone("gg", "VarSnippet");
    Snippet *p = manager->getone("p", "VarSnippet");
    Snippet *longvar = manager->getone("longvar", "VarSnippet");
    Snippet *long_var = manager->getone("long_var", "VarSnippet");
    Snippet *longvarinit = manager->getone("longvarinit", "VarSnippet");
    
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
    // test decl
    SnippetManager *manager = new SnippetManager();
    manager->add(createSnippets(file_decl_h));
    manager->process();

    Snippet *A = manager->getone("A", "RecordDeclSnippet");
    Snippet *B = manager->getone("B", "RecordDeclSnippet");
    Snippet *foo = manager->getone("foo", "FunctionDeclSnippet");
    Snippet *bar = manager->getone("bar", "FunctionDeclSnippet");

    ASSERT_TRUE(A && B && foo && bar);

    EXPECT_EQ(A->getCode(), "struct A");
    EXPECT_EQ(B->getCode(), "struct B");
    EXPECT_EQ(foo->getCode(), "int foo()");
    EXPECT_EQ(bar->getCode(), "int bar()");
  }

  {
    // test dependency
    SnippetManager *manager = new SnippetManager();
    manager->add(createSnippets(file_a_c));
    manager->add(createSnippets(file_a_h));
    manager->add(createSnippets(file_b_c));
    manager->add(createSnippets(file_b_h));
    manager->add(createSnippets(file_sub_c));
    manager->add(createSnippets(file_lib_c));
    manager->process();

    Snippet *A1 = manager->getone("A1", "RecordSnippet");
    Snippet *A2 = manager->getone("A2", "RecordSnippet");
    Snippet *A2Typedef = manager->getone("A2", "TypedefSnippet");
    Snippet *A2Decl = manager->getone("A2", "RecordDeclSnippet");
    Snippet *A3 = manager->getone("A3", "RecordSnippet");
    Snippet *A3Alias = manager->getone("A3Alias", "TypedefSnippet");
    
    Snippet *global_a = manager->getone("global_a", "VarSnippet");
    Snippet *temp_a = manager->getone("temp_a", "VarSnippet");
    Snippet *myvar = manager->getone("myvar", "VarSnippet");
    Snippet *aoo = manager->getone("aoo", "FunctionSnippet");


    // manager->dump(std::cout);
    
    std::set<Snippet*> dep;
    dep = A1->getDeps();
    EXPECT_EQ(dep.size(), 0);
    dep = A2->getDeps();
    EXPECT_EQ(dep.size(), 1);
    EXPECT_EQ(dep.count(A1), 1);
    dep = A3->getDeps();
    EXPECT_EQ(dep.size(), 3);
    EXPECT_EQ(dep.count(A2), 1);
    EXPECT_EQ(dep.count(A2Typedef), 1);
    EXPECT_EQ(dep.count(A2Decl), 1);

    // get all dependence
    dep = A3->getAllDeps();
    EXPECT_EQ(dep.size(), 4);
    EXPECT_EQ(dep.count(A1), 1);
    EXPECT_EQ(dep.count(A2), 1);
    EXPECT_EQ(dep.count(A2Typedef), 1);
    EXPECT_EQ(dep.count(A2Decl), 1);

    // outer
    std::set<Snippet*> outer;
    // manager->dump(std::cout);
    // manager->dumpSnippetsVerbose(std::cout);
    outer = A3->getOuters();
    EXPECT_EQ(outer.size(), 1);
    EXPECT_EQ(outer.count(A3Alias), 1);

    // TODO practical usage
  }

  {
    // Test save and load back
    std::vector<Snippet*> snippets = createSnippets(file_var_c);
    SnippetManager *manager = new SnippetManager();
    manager->add(snippets);
    manager->process();

    fs::path random_json = fs::unique_path(fs::temp_directory_path() / "%%%%-%%%%-%%%%-%%%%.json");

    manager->saveJson(random_json);
    // std::cout << "Saved to " << random_json << "\n";

    // a new manager to load
    SnippetManager *manager2 = new SnippetManager();
    manager2->loadJson(random_json);

    // verify
    std::vector<Snippet*> snippets2 = manager2->getSnippets();
    // check snippet and snippet2
    ASSERT_EQ(snippets.size(), snippets2.size());
    for (int i=0;i<snippets.size();i++) {
      Snippet *s1 = snippets[i];
      Snippet *s2 = snippets2[i];
      EXPECT_EQ(s1->getName(), s2->getName());
      // dep and outers
      EXPECT_EQ(s1->getDepsAsId(), s2->getDepsAsId());
      EXPECT_EQ(s1->getOutersAsId(), s2->getOutersAsId());
    }
  }
}
