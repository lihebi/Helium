#include "helium/parser/source_manager.h"

#include "helium/parser/parser.h"
#include "helium/utils/string_utils.h"

using namespace v2;

using std::string;
using std::vector;
using std::set;
using std::map;
using std::pair;

SourceManager::SourceManager(fs::path cppfolder) : cppfolder(cppfolder) {
  fs::recursive_directory_iterator it(cppfolder), eod;
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (p.extension() == ".c") {
      std::cout << "[SourceManager] " << "parsing " << p.string() << "\n";
      Parser *parser = new Parser(p.string());
      ASTContext *ast = parser->getASTContext();
      ast->setSourceManager(this);
      // ASTs.push_back(ast);
      // files.push_back(p);
      File2ASTMap[p] = ast;
      AST2FileMap[ast] = p;
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
  for (auto m : File2ASTMap) {
    ASTContext *ast = m.second;
    std::cout << "== AST:" << "\n";
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    std::ostringstream os;
    Printer *printer = new Printer(os);
    printer->visit(unit);
    std::cout << Printer::PrettyPrint(os.str()) << "\n";
  }
}


fs::path SourceManager::matchFile(fs::path file) {
  fs::path ret;
  fs::path remaining;
  for (auto m : File2ASTMap) {
    fs::path f = m.first;
    // compare filename and move to parent
    // see how many times it is compared
    fs::path relf = fs::relative(f, cppfolder);
    fs::path file1 = file;
    fs::path file2 = relf;
    while (file1.filename() == file2.filename()) {
      file1 = file1.parent_path();
      file2 = file2.parent_path();
    }
    if (!file1.empty()) continue;
    // now we found a match
    if (ret.empty()) {
      ret = f;
      remaining = file2;
    } else if (remaining.size() > file2.size()) {
      remaining = file2;
      ret = f;
    }
  }
  return ret;
}

void SourceManager::select(std::map<std::string, std::set<std::pair<int,int> > > selection) {
  // TODO lcoate which AST(s)
  for (auto sel : selection) {
    fs::path file = matchFile(sel.first);
    if (!file.empty()) {
      ASTContext *ast = File2ASTMap[file];
      TokenVisitor *tokenVisitor = new TokenVisitor();
      // Create Token Visitor for that AST
      TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
      tokenVisitor->visit(unit);
      // apply and select the tokens based on source range
      vector<v2::ASTNodeBase*> tokens = tokenVisitor->getTokens();
      map<v2::ASTNodeBase*,int> idmap = tokenVisitor->getIdMap();
      for (const pair<int,int> &p : sel.second) {
        int line = p.first;
        int column = p.second;
        SourceLocation loc(line, column);
        // I know this is inefficient
        for (v2::ASTNodeBase *token : tokens) {
          SourceLocation begin = token->getBeginLoc();
          SourceLocation end = token->getEndLoc();
          // std::cout << loc << " === " << begin << " -- " << end << " ";

          // std::ostringstream os;
          // Printer printer(os);
          // token->accept(&printer);

          // std::cout << os.str() << "\n";
          
          if (begin <= loc && loc <= end) {
            // this token is selected
            // print it out for now
            std::ostringstream os;
            Printer printer(os);
            token->accept(&printer);
            std::cout << "Found selected token: " << os.str() << "\n";
            this->selection.insert(token);
          }
        }
      }
    }
  }
  // maybe here: get/set the distribution information
  std::cout << "Total selected tokens: " << this->selection.size() << "\n";
}

void SourceManager::grammarPatch() {
  std::cout << "Doing grammar patching on " << selection.size() << " selected tokens .." << "\n";
}
