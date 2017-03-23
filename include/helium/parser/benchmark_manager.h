#ifndef BENCHMARK_MANAGER_H
#define BENCHMARK_MANAGER_H

#include <vector>
#include "helium/parser/ast_v2.h"
#include "helium/utils/common.h"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <gtest/gtest.h>

namespace fs = boost::filesystem;


class BenchmarkManager {
public:
  /**
   * Precondition: p should be the cache/XXX/cpp folder
   */
  BenchmarkManager(fs::path cppfolder);
  ~BenchmarkManager() {}
  // void processProject(fs::path cppfolder);
  v2::ASTNodeBase *getNodeById(int id) {
    return Nodes[id];
  }
  int getIdByNode(v2::ASTNodeBase *node) {
    return IDs[node];
  }
  void dumpTokens();
private:
  // this class holds all ASTs
  // it also determines the ID of ast nodes
  // each AST node should have an unique ID
  std::vector<v2::ASTContext*> ASTs;
  std::vector<v2::ASTNodeBase*> Nodes;
  std::map<v2::ASTNodeBase*,int> IDs;
};


#endif /* BENCHMARK_MANAGER_H */
