#include "helium/parser/source_manager.h"

#include "helium/parser/parser.h"

using namespace v2;

SourceManager::SourceManager(fs::path cppfolder) {
  fs::recursive_directory_iterator it(cppfolder), eod;
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (p.extension() == ".c") {
      std::cout << "parsing " << p.string() << "\n";
      Parser *parser = new Parser(p.string());
      ASTContext *ast = parser->getASTContext();
      ast->setSourceManager(this);
      ASTs.push_back(ast);
      // v2::TranslationUnitDecl *unit = parser->getTranslationUnit();
      // if (unit) {
      //   unit->dump();
      // }
      // std::cout << "should output" << "\n";
    }
  }
  // extract all nodes, and assign ids

  // should not get nodes from ASTContext directly
  // Instead, use a visitor
  //
  // for (ASTContext *ast : ASTs) {
  //   std::vector<ASTNodeBase*> nodes = ast->getNodes();
  //   Nodes.insert(Nodes.end(), nodes.begin(), nodes.end());
  // }
  // keep the map of node -> ID
  // for (int i=0;i<Nodes.size();i++) {
  //   IDs[Nodes[i]] = i;
  // }

  // for (ASTContext *ast : ASTs) {
  //   LevelVisitor *levelVisitor = new LevelVisitor();
  //   TranslationUnitDecl *decl = ast->getTranslationUnitDecl();
  //   levelVisitor->visit(decl);
  //   std::map<v2::ASTNodeBase*, int> levels = levelVisitor->getLevels();
  //   // examine levels
  // }
}

// void SourceManager::dumpTokens() {
//   for (int i=0;i<Nodes.size();i++) {
//     std::cout << i << " : " << Nodes[i]->label() << "\n";
//   }
// }

void SourceManager::dumpASTs() {
  for (ASTContext *ast : ASTs) {
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    Printer *printer = new Printer(std::cout);
    printer->visit(unit);
  }
}

