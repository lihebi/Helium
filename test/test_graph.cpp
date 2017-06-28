#include "helium/utils/Graph.h"
#include <iostream>

#include <gtest/gtest.h>

class MyData {
public:
  MyData(std::string s) : name(s) {}
  void dump(std::ostream &os) {
    os << name;
  }
private:
  std::string name;
};

TEST(GraphTest, MyTest) {
  hebigraph::Graph<MyData*> g;
  MyData *a = new MyData("a");
  MyData *b = new MyData("b");
  MyData *c = new MyData("c");
  MyData *d = new MyData("d");
  
  g.addNode(a);
  g.addNode(b);
  g.addNode(c);
  g.addNode(d);

  g.addEdge(a,b);
  g.addEdge(b,c);

  // g.dump(std::cout);

  EXPECT_TRUE(g.hasNode(a));
  EXPECT_TRUE(g.hasNode(b));
  EXPECT_TRUE(g.hasNode(c));
  EXPECT_TRUE(g.hasNode(d));

  g.removeNode(d);
  EXPECT_FALSE(g.hasNode(d));

  std::vector<MyData*> l = g.topoSort();
  ASSERT_EQ(l.size(), 3);
  EXPECT_EQ(l[0], a);
  EXPECT_EQ(l[1], b);
  EXPECT_EQ(l[2], c);

  EXPECT_FALSE(g.hasCycle());
  g.addEdge(c,a);
  EXPECT_TRUE(g.hasCycle());
}


TEST(GraphTest, StringTest) {
  hebigraph::Graph<std::string> g;
  std::string a("a");
  std::string b("b");
  std::string c("c");
  std::string d("d");

  g.addNode(a);
  g.addNode(b);
  g.addNode(c);
  g.addNode(d);

  g.addEdge(a,b);
  g.addEdge(b,c);

  // g.dump(std::cout);

  EXPECT_TRUE(g.hasNode(a));
  EXPECT_TRUE(g.hasNode(b));
  EXPECT_TRUE(g.hasNode(c));
  EXPECT_TRUE(g.hasNode(d));

  g.removeNode(d);
  EXPECT_FALSE(g.hasNode(d));

  std::vector<std::string> l = g.topoSort();
  ASSERT_EQ(l.size(), 3);
  EXPECT_EQ(l[0], a);
  EXPECT_EQ(l[1], b);
  EXPECT_EQ(l[2], c);

  EXPECT_FALSE(g.hasCycle());
  g.addEdge(c,a);
  EXPECT_TRUE(g.hasCycle());
}
