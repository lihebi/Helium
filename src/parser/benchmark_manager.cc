#include "helium/parser/benchmark_manager.h"

#include "helium/parser/parser.h"

using namespace v2;

BenchmarkManager::BenchmarkManager(fs::path cppfolder) {
  fs::recursive_directory_iterator it(cppfolder), eod;
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (p.extension() == ".c") {
      std::cout << "parsing " << p.string() << "\n";
      Parser *parser = new Parser(p.string());
      ASTContext *ast = parser->getASTContext();
      ASTs.push_back(ast);
      // v2::TranslationUnitDecl *unit = parser->getTranslationUnit();
      // if (unit) {
      //   unit->dump();
      // }
      // std::cout << "should output" << "\n";
    }
  }
  // extract all nodes, and assign ids
  for (ASTContext *ast : ASTs) {
    std::vector<ASTNodeBase*> nodes = ast->getNodes();
    Nodes.insert(Nodes.end(), nodes.begin(), nodes.end());
  }
  // keep the map of node -> ID
  for (int i=0;i<Nodes.size();i++) {
    IDs[Nodes[i]] = i;
  }
}

void BenchmarkManager::dumpTokens() {
  for (int i=0;i<Nodes.size();i++) {
    std::cout << i << " : " << Nodes[i]->label() << "\n";
  }
}

