#include "dot.h"
#include <gtest/gtest.h>


TEST(DotGraphTestCase, DotGraphTest) {
  DotGraph dotgraph;
  dotgraph.AddNode("id_1", "label 1");
  dotgraph.AddNode("id_2", "label 2");
  dotgraph.AddEdge("id_1", "id_2");
  std::string dot = dotgraph.dump();
  // std::cout << dot  << "\n";
}
