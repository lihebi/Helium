#include "helium/parser/source_manager.h"

#include "helium/parser/parser.h"
#include "helium/utils/string_utils.h"

#include "helium/utils/rand_utils.h"

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

void SourceManager::grammarPatch() {
  std::cout << "Doing grammar patching on " << selection.size() << " selected tokens .." << "\n";
  for (auto &m : File2ASTMap) {
    ASTContext *ast = m.second;
    StandAloneGrammarPatcher *patcher = new StandAloneGrammarPatcher(ast, selection);
    patcher->process();
    set<ASTNodeBase*> patch = patcher->getPatch();
    // FIXME examine the result
  }
}


std::set<v2::ASTNodeBase*> SourceManager::generateRandomSelection() {
  // Here I can enforce some criteria, such as intro-procedure
  set<ASTNodeBase*> ret;
  for (auto &m : File2ASTMap) {
    ASTContext *ast = m.second;
    TokenVisitor *tokenVisitor = new TokenVisitor();
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    unit->accept(tokenVisitor);
    vector<ASTNodeBase*> tokens = tokenVisitor->getTokens();
    // random select some tokens
    // vector<ASTNodeBase*> token_vector(tokens.begin(), tokens.end());
    // first, random get # of tokens
    // Then, random get tokens
    int num = utils::rand_int(0, tokens.size());
    while (ret.size() < num) {
      int idx = utils::rand_int(0, tokens.size());
      ret.insert(tokens[idx]);
    }
  }
  return ret;
}

std::string SourceManager::getTokenUUID(v2::ASTNodeBase* node) {
  ASTContext *ast = node->getASTContext();
  std::string ret;
  if (AST2FileMap.count(ast) == 0) {
    return "";
  }
  fs::path file = AST2FileMap[ast];
  ret += file.string();
  TokenVisitor *tokenVisitor = AST2TokenVisitorMap[ast];
  int id = tokenVisitor->getId(node);
  ret += "_" + std::to_string(id);
  return ret;
}

fs::path SourceManager::getTokenFile(v2::ASTNodeBase* node) {
  ASTContext *ast = node->getASTContext();
  std::string ret;
  if (AST2FileMap.count(ast) == 1) {
    return AST2FileMap[ast];
  }
  return fs::path("");
}
int SourceManager::getTokenId(v2::ASTNodeBase* node) {
  ASTContext *ast = node->getASTContext();
  if (AST2TokenVisitorMap.count(ast) != 0) {
    TokenVisitor *tokenVisitor = AST2TokenVisitorMap[ast];
    return tokenVisitor->getId(node);
  }
  return -1;
}



std::set<v2::ASTNodeBase*> SourceManager::loadSelection(fs::path sel_file) {
  map<string, set<pair<int,int> > > selection;
  if (fs::exists(sel_file)) {
    // this is a list of IDs
    std::ifstream is;
    is.open(sel_file.string());
    if (is.is_open()) {
      // int line,column;
      // while (is >> line >> column) {
      //   selection.insert(std::make_pair(line, column));
      // }
      std::string line;
      std::string file;
      set<int> sel;
      while (std::getline(is, line)) {
        utils::trim(line);
        if (line.empty()) {
          continue;
        } else if (line[0] == '#') {
          file = line.substr(1);
          utils::trim(file);
        } else {
          vector<string> v = utils::split(line);
          // the third is ID, not used here
          if (v.size() >= 2) {
            selection[file].insert(std::make_pair(stoi(v[0]), stoi(v[1])));
          }
        }
      }
    }
  }

  set<ASTNodeBase*> ret;
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
            ret.insert(token);
          }
        }
      }
    }
  }
  // maybe here: get/set the distribution information
  std::cout << "Total selected tokens: " << ret.size() << "\n";
  return ret;
}


void SourceManager::dumpSelection(std::ostream &os, std::set<v2::ASTNodeBase*> selection) {
  // dump to os
  for (auto &m : AST2TokenVisitorMap) {
    ASTContext *ast = m.first;
    TokenVisitor *tokenVisitor = m.second;
    fs::path file = AST2FileMap[ast];
    // output filename
    os << "# " << file.string() << "\n";
    vector<ASTNodeBase*> tokens = tokenVisitor->getTokens();
    for (ASTNodeBase *token : tokens) {
      if (selection.count(token) == 1) {
        SourceLocation loc = token->getBeginLoc();
        os << loc.getLine() << " " << loc.getColumn() << " " << tokenVisitor->getId(token) << "\n";
      }
    }
  }
}
