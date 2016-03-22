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

TEST(segment_test_case, get_to_resolve_test) {
  Doc doc;
  const char* raw = R"prefix(

// @Helium
mystruct a;
a = func(b);
a = NULL; // c common will not be in.
/* also, comments will not in either */

)prefix";
  utils::string2xml(raw, doc);
  Node comment = ast::find_node_containing_str(doc, NK_Comment, "@Helium");
  NodeList nodes;
  Node n = next_sibling(comment);
  nodes.push_back(n);
  nodes.push_back(next_sibling(n));

  std::set<std::string> to {"test"};
  std::set<std::string> no {"a", "b"};
  std::set<std::string> result = get_to_resolve(nodes, to, no);
  ASSERT_EQ(result.size(), 3);
}


/**
 * Disabled because the population is for visualization purpose.
 */
TEST(segment_test_case, DISABLED_population) {
  Doc doc;
  const char* raw = R"prefix(

int foo() {
mystruct a;
a = func(b);
a = NULL;
}

int bar(int a, char *b) {
  int x;
  int y;
  int sum;
  int n;
  for (int i=0;i<n;i++) {
    if (i %2 == 0) {
      sum += i;
    } else {
      sum += x * y;
    }
  }
}

void foobar () {
if (x>0) {
  while (x<10) {
    a=b;
    c=d;
    if (a>c) {
      sum+=c;
    } else if (a==c) {
      sum += con1;
    } else {
      sum += a;
    }
  }
} else {
  sum = 0;
  for (int i=0;i<8;i++) {
    sum += i;
  }
}
}


)prefix";
  utils::string2xml(raw, doc);
  NodeList nodes = ast::find_nodes(doc, NK_Function);
  ASSERT_EQ(nodes.size(), 3);
  AST *ast1 = new AST(nodes[0]);
  AST *ast2 = new AST(nodes[1]);
  AST *ast3 = new AST(nodes[2]);
  // ast1.Load(nodes[0]);
  // ast2.Load(nodes[1]);
  // ast3.Load(nodes[2]);
  
  // AST 1
  // std::cout <<ast1.GetSigStr()  << "\n";
  // AST 2
  // std::cout <<ast2.GetSigStr()  << "\n";
  // AST 3
  // std::cout <<ast.GetSigStr()  << "\n";
  Individual ind;
  ind.SetAST(ast3);
  // int size = ind.size();
  std::vector<int> gene;
  utils::seed_rand();

  ast1->Visualize({}, "random", true);
  std::cout <<ast1->GetSigStr()  << "\n";
  ast2->Visualize({}, "random", true);
  std::cout <<ast2->GetSigStr()  << "\n";
  // gene = rand_gene(size);
  // ind.SetGene(gene);
  // ind.Visualize("tmp/1", "ps", false);

  // ind.SetGene(gene);
  // std::cout << "gene: ";
  // for (int g : gene) {
  //   std::cout << g;
  // }
  // std::cout << "\n";

  // for (int i=0;i<8;i++) {
  //   gene = rand_gene(size);
  //   ind.SetGene(gene);
  //   std::string dir = "tmp/"+std::to_string(i);
  //   ind.Visualize(dir, false);
  //   std::string spec = "Gene: "+ind.GetGeneStr() + "\n"
  //     + "Complete Gene: " + ind.GetCGeneStr() + "\n"
  //     + "AST Signature: " + ind.GetASTSigStr();
  //   std::cout <<spec  << "\n";
  //   utils::write_file(dir+"/spec.txt", spec);
  // }

  // gene = rand_gene(size);
  // ind.SetGene(gene);
  // ind.Visualize("tmp/2", "ps", false);
  
  // gene = rand_gene(size);
  // ind.SetGene(gene);
  // ind.Visualize("tmp/3", "ps", false);
  
  // gene = rand_gene(size);
  // ind.SetGene(gene);
  // ind.Visualize("tmp/4", "ps", false);

  delete ast1;
  delete ast2;
  delete ast3;
}

