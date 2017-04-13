#include "helium/parser/source_manager.h"

#include "helium/parser/parser.h"
#include "helium/utils/string_utils.h"

#include "helium/utils/rand_utils.h"
#include "helium/parser/GrammarPatcher.h"

#include "helium/resolver/SnippetV2.h"

#include <regex>

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

  // std::map<v2::ASTContext*, TokenVisitor*> AST2TokenVisitorMap;
  // std::map<v2::ASTContext*, Distributor*> AST2DistributorMap;

  for (auto &m : File2ASTMap) {
    ASTContext *ast = m.second;
    TokenVisitor *tokenVisitor = new TokenVisitor();
    Distributor *distributor = new Distributor();
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    unit->accept(tokenVisitor);
    unit->accept(distributor);
    AST2TokenVisitorMap[ast] = tokenVisitor;
    AST2DistributorMap[ast] = distributor;
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



// void SourceManager::dumpASTs() {
//   for (auto m : File2ASTMap) {
//     ASTContext *ast = m.second;
//     std::cout << "== AST:" << "\n";
//     TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
//     std::ostringstream os;
//     Printer *printer = new Printer(os);
//     printer->visit(unit);
//     std::cout << Printer::PrettyPrint(os.str()) << "\n";
//   }
// }

// void SourceManager::dumpTokens() {
//   for (auto &m : File2ASTMap) {
//     ASTContext *ast = m.second;
//     TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
//     TokenDumper dumper(std::cout);
//     unit->accept(&dumper);
//   }
// }

void SourceManager::dumpASTs() {
  for (auto &m : File2ASTMap) {
    ASTContext *ast = m.second;
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    std::ostringstream os;
    Printer printer;
    unit->accept(&printer);
    std::cout << printer.getString() << "\n";
  }
}


/**
 * Match in the File2ASTMap to find the best match
 * TODO test this
 */
fs::path SourceManager::matchFile(fs::path file) {
  fs::path ret;
  fs::path remaining;
  for (auto m : File2ASTMap) {
    fs::path f = m.first;
    if (utils::match_suffix(f, file)) return f;
  }
  return ret;
}

std::set<ASTNodeBase*> SourceManager::grammarPatch(std::set<ASTNodeBase*> sel) {
  std::cout << "[SourceManager] Doing grammar patching on " << sel.size() << " selected tokens .." << "\n";
  std::set<ASTNodeBase*> ret = sel;
  for (auto &m : File2ASTMap) {
    ASTContext *ast = m.second;
    fs::path file = m.first;
    // std::cout << "Patching for " << file.string() << "\n";
    
    // TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    // std::ostringstream oss;
    // Printer *printer = new Printer(oss);
    // unit->accept(printer);
    // std::cout << Printer::PrettyPrint(oss.str()) << "\n";

    
    StandAloneGrammarPatcher *patcher = new StandAloneGrammarPatcher(ast, sel);
    patcher->process();
    set<ASTNodeBase*> patch = patcher->getPatch();
    // FIXME examine the result
    ret.insert(patch.begin(), patch.end());
  }
  return ret;
}


std::set<v2::ASTNodeBase*> SourceManager::defUse(std::set<v2::ASTNodeBase*> sel) {
  std::map<v2::ASTNodeBase*,std::set<v2::ASTNodeBase*> > use2def;
  for (auto &m : File2ASTMap) {
    ASTContext *ast = m.second;
    fs::path file = m.first;
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    SymbolTableBuilder *symbolTableBuilder = new SymbolTableBuilder();
    unit->accept(symbolTableBuilder);
    std::map<v2::ASTNodeBase*,std::set<v2::ASTNodeBase*> > u2d = symbolTableBuilder->getUse2DefMap();
    use2def.insert(u2d.begin(), u2d.end());
  }
  // process sel
  // this needs to be recursive
  std::set<v2::ASTNodeBase*> ret = sel;

  std::vector<v2::ASTNodeBase*> worklist(sel.begin(), sel.end());
  std::set<v2::ASTNodeBase*> done;
  while (!worklist.empty()) {
    v2::ASTNodeBase *node = worklist.back();
    worklist.pop_back();
    done.insert(node);
    if (use2def.count(node) == 1) {
      std::set<ASTNodeBase*> tmp = use2def[node];
      ret.insert(tmp.begin(), tmp.end());
      for (auto *n : tmp) {
        if (done.count(n) == 0) worklist.push_back(n);
      }
    }
  }
  // for (v2::ASTNodeBase *node : sel) {
  //   if (use2def.count(node) == 1) {
  //     std::set<ASTNodeBase*> tmp = use2def[node];
  //     ret.insert(tmp.begin(), tmp.end());
  //   }
  // }
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



/**
 * Selection
 */


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

std::set<v2::ASTNodeBase*> get_rand(std::vector<v2::ASTNodeBase*> nodes, int num) {
  std::set<v2::ASTNodeBase*> ret;
  // return all nodes or simply empty?
  if (num > nodes.size()) return ret;
  std::set<int> idxes = utils::rand_ints(0, nodes.size()-1, num);
  for (int idx : idxes) {
    ret.insert(nodes[idx]);
  }
  return ret;
}

std::set<v2::ASTNodeBase*> SourceManager::genRandSel(int num) {
  // TODO get all tokens only one time to get performance
  std::vector<v2::ASTNodeBase*> nodes;
  // TODO get only tokens inside functions
  // TODO get only tokens inside same functions
  for (auto &m : File2ASTMap) {
    ASTContext *ast = m.second;
    TokenVisitor *tokenVisitor = new TokenVisitor();
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    unit->accept(tokenVisitor);
    vector<ASTNodeBase*> tokens = tokenVisitor->getTokens();
    nodes.insert(nodes.end(), tokens.begin(), tokens.end());
  }
  return get_rand(nodes, num);
}

/**
 * the tokens must be in functions
 */
std::set<v2::ASTNodeBase*> SourceManager::genRandSelFunc(int num) {
  std::vector<v2::ASTNodeBase*> functions;
  for (auto &m : AST2DistributorMap) {
    Distributor *dist = m.second;
    std::set<ASTNodeBase*> funcs = dist->getFuncNodes();
    functions.insert(functions.end(), funcs.begin(), funcs.end());
  }
  std::vector<v2::ASTNodeBase*> nodes;
  for (v2::ASTNodeBase* func : functions) {
    TokenVisitor visitor;
    func->accept(&visitor);
    vector<ASTNodeBase*> tokens = visitor.getTokens();
    nodes.insert(nodes.end(), tokens.begin(), tokens.end());
  }
  return get_rand(nodes, num);
}

/**
 * the tokens must be in the same function
 */
std::set<v2::ASTNodeBase*> SourceManager::genRandSelSameFunc(int num) {
  // get a vector of vector of nodes, ordered by function
  std::vector<v2::ASTNodeBase*> functions;
  for (auto &m : AST2DistributorMap) {
    Distributor *dist = m.second;
    std::set<ASTNodeBase*> funcs = dist->getFuncNodes();
    functions.insert(functions.end(), funcs.begin(), funcs.end());
  }
  // filter out functions with less tokens
  std::vector<ASTNodeBase*> tmp;
  for (ASTNodeBase *node : functions) {
    TokenVisitor visitor;
    node->accept(&visitor);
    vector<ASTNodeBase*> tokens = visitor.getTokens();
    if (tokens.size() >= num) tmp.push_back(node);
  }
  functions = tmp;
  // get the function to use
  if (functions.size() == 0) return {};
  int func_idx = utils::rand_int(0, functions.size()-1);
  ASTNodeBase *func = functions[func_idx];
  // get tokens
  TokenVisitor visitor;
  func->accept(&visitor);
  vector<ASTNodeBase*> tokens = visitor.getTokens();
  return get_rand(tokens, num);
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
      // first line must be # filename.c


      // anything starts with #file will change the file
      // anything starts with # will be comment
      // anything else should starts with two numbers representing line and column, starting from 1
      
      while (std::getline(is, line)) {
        utils::trim(line);
        if (line.empty()) continue;
        else if (line[0] == '#') {
          // control directive
          std::string first = utils::split(line)[0];
          if (first == "#file") {
            file = utils::split(line)[1];
          }
        } else {
          vector<string> v = utils::split(line);
          assert(!file.empty());
          // Only first two are used. The third can be any comment
          if (v.size() >= 2) {
            int l = stoi(v[0]);
            int c = stoi(v[1]);
            selection[file].insert(std::make_pair(l,c));
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
            std::cout << "[SourceManager] Found selected token: ";
            token->dump(std::cout);
            std::cout << "\n";
            ret.insert(token);
          }
        }
      }
    }
  }
  // maybe here: get/set the distribution information
  // std::cout << "Total selected tokens: " << ret.size() << "\n";
  return ret;
}


void SourceManager::dumpSelection(std::set<v2::ASTNodeBase*> selection, std::ostream &os) {
  // dump to os
  for (auto &m : AST2TokenVisitorMap) {
    ASTContext *ast = m.first;
    TokenVisitor *tokenVisitor = m.second;
    fs::path file = AST2FileMap[ast];
    // output filename
    os << "#file " << file.string() << "\n";
    vector<ASTNodeBase*> tokens = tokenVisitor->getTokens();
    for (ASTNodeBase *token : tokens) {
      if (selection.count(token) == 1) {
        SourceLocation loc = token->getBeginLoc();
        os << loc.getLine() << " " << loc.getColumn() << "\n";
        // << " " << tokenVisitor->getId(token) << "\n";
      }
    }
  }
}

typedef struct Distribution {
  int tok;
  int patch;
  int file;
  double per_file;
  int proc;
  double per_proc;
  int IF;
  double per_IF;
  int loop;
  double per_loop;
  int SWITCH;
  double per_SWITCH;
  bool result;
  void dump(std::ostream &os) {
    os << "#tok,#patch,"
       << "#file,#per(file),#proc,#per(proc),#if,#per(if),"
       << "#loop,#per(loop),#switch,#per(switch),"
       << "#res" << "\n";
    os << tok << "," << patch << ","
       << file << "," << per_file << "," << proc << "," << per_proc << "," << IF << "," << per_IF << ","
       << loop << "," << per_loop << "," << SWITCH << "," << per_SWITCH << ","
       << result << "\n";
  }
} Distribution;

// #tok | #patch |
// #file | #per(file) | #proc | #per(proc) | #if | #per(if)
// #loop | #per(loop) | # switch | #per(switch)
// result
void SourceManager::analyzeDistribution(std::set<v2::ASTNodeBase*> selection,
                                        std::set<v2::ASTNodeBase*> patch,
                                        std::ostream &os) {
  Distribution dist = {0};
  // TODO populate these information
  dist.tok = selection.size();
  dist.patch = patch.size();
  dist.file = getDistFile(selection);
  dist.per_file = (double)dist.tok / dist.file;
  dist.proc = getDistProc(selection);
  dist.per_proc = (double)dist.tok / dist.proc;
  dist.IF = getDistIf(selection);
  dist.per_IF = (double)dist.tok / dist.IF;
  dist.loop = getDistLoop(selection);
  dist.per_loop = (double)dist.tok / dist.loop;
  dist.SWITCH = getDistSwitch(selection);
  dist.per_SWITCH = (double)dist.tok / dist.SWITCH;
  // TODO result
  // dist.result;
  dist.dump(os);
}


// TODO use the visitor "distributor" to compute the distribution database!
int SourceManager::getDistFile(set<ASTNodeBase*> sel) {
  set<ASTContext*> asts;
  for (auto *node : sel) {
    asts.insert(node->getASTContext());
  }
  return asts.size();
}
int SourceManager::getDistProc(set<ASTNodeBase*> sel) {
  int ret = 0;
  for (auto &m : AST2DistributorMap) {
    // OMG Proc vs Func the name is not consistent ..
    int num = m.second->getDistFunc(sel);
    ret += num;
  }
  return ret;
}
int SourceManager::getDistIf(set<ASTNodeBase*> sel) {
  int ret=0;
  for (auto &m : AST2DistributorMap) {
    int num = m.second->getDistIf(sel);
    ret += num;
  }
  return ret;
}
int SourceManager::getDistLoop(set<ASTNodeBase*> sel) {
  int ret=0;
  for (auto &m : AST2DistributorMap) {
    int num = m.second->getDistLoop(sel);
    ret += num;
  }
  return ret;
}
int SourceManager::getDistSwitch(set<ASTNodeBase*> sel) {
  int ret=0;
  for (auto &m : AST2DistributorMap) {
    int num = m.second->getDistSwitch(sel);
    ret += num;
  }
  return ret;
}






std::set<v2::ASTContext*> SourceManager::getASTByNodes(std::set<v2::ASTNodeBase*> nodes) {
  std::set<v2::ASTContext*> ret;
  for (auto *node : nodes) {
    ret.insert(node->getASTContext());
  }
  return ret;
}
/**
 * If only selection across multiple functions, select header for all of them.
 * This includes:
 * - return node
 * - name node
 * - param node
 * - compount stmt
 * - comp token
 *
 * Also, TODO if the selection contains return xxx;, we should not put into main function.
 * This might cause compile error.
 */
std::set<v2::ASTNodeBase*> SourceManager::patchFunctionHeader(std::set<v2::ASTNodeBase*> sel) {
  int func_ct = 0;
  for (auto &m : AST2DistributorMap) {
    // ASTContext *ast = m.first;
    Distributor *dist = m.second;
    func_ct += dist->getDistFunc(sel);
  }
  if (func_ct == 0) return sel;
  // TODO detect if return is selected
  else if (func_ct == 1) return sel;
  else {
    std::set<v2::ASTContext*> asts = getASTByNodes(sel);
    std::set<v2::ASTNodeBase*> funcs;
    for (v2::ASTContext *ast : asts) {
      Distributor *dist = AST2DistributorMap[ast];
      std::set<v2::ASTNodeBase*> nodes = dist->getDistFuncNodes(sel);
      funcs.insert(nodes.begin(), nodes.end());
    }
    // now i got all the functiondecl nodes
    // add the relevant nodes into the selection
    for (auto *node : funcs) {
      FunctionDecl *func = dynamic_cast<FunctionDecl*>(node);
      if (func) {
        // FIXME this does not going through grammar patching!!
        sel.insert(func);
        sel.insert(func->getReturnTypeNode());
        sel.insert(func->getNameNode());
        sel.insert(func->getParamNode());
        sel.insert(func->getBody());
        CompoundStmt *body = dynamic_cast<CompoundStmt*>(func->getBody());
        sel.insert(body->getCompNode());
      }
    }
  }
  return sel;
}

bool SourceManager::shouldPutIntoMain(std::set<v2::ASTNodeBase*> sel) {
  int func_ct = 0;
  for (auto *node : sel) {
    if (dynamic_cast<FunctionDecl*>(node)) return false;
  }
  for (auto &m : AST2DistributorMap) {
    // ASTContext *ast = m.first;
    Distributor *dist = m.second;
    func_ct += dist->getDistFunc(sel);
  }
  // no function is selected, definitely put into main function
  // this should be a global variable definition
  if (func_ct == 0) return true;
  if (func_ct == 1) return true;
  return false;
  
}

std::string SourceManager::generateProgram(std::set<v2::ASTNodeBase*> sel) {
  // analyze distribution
  // std::map<v2::ASTContext*, Distributor*> AST2DistributorMap;
  sel = patchFunctionHeader(sel);

  std::string ret;
  // ret += "#include <stdio.h>\n";
  // ret += "#include <stdlib.h>\n";
  // ret += "#include <string.h>\n";
  ret += "#include \"main.h\"\n";
  
  if (shouldPutIntoMain(sel)) {
    std::string body;
    for (auto &m : File2ASTMap) {
      Generator *generator =  new Generator();
      generator->setSelection(sel);
      generator->adjustReturn(true);
      // generator->setSelection(sel);
      ASTContext *ast = m.second;
      TranslationUnitDecl *decl = ast->getTranslationUnitDecl();
      decl->accept(generator);
      std::string prog = generator->getProgram();
      body += prog;
    }
    ret += "// Should into main\n";
    ret += "int main(int argc, char *argv[]) {\n";
    ret += body;
    ret += "  return 0;\n";
    ret += "}\n";
  } else {
    std::string body;
    // sel = patchFunctionHeader(sel);
    for (auto &m : File2ASTMap) {
      Generator *generator =  new Generator();
      generator->setSelection(sel);
      // generator->setSelection(sel);
      ASTContext *ast = m.second;
      TranslationUnitDecl *decl = ast->getTranslationUnitDecl();
      decl->accept(generator);
      std::string prog = generator->getProgram();
      body += prog;
    }
    ret += body;

    ret += "// Should NOT into main\n";
    ret += "int main(int argc, char *argv[]) {\n";
    // TODO call to those functions
    ret += "  return 0;\n";
    ret += "}\n";
  }
  return ret;
}





std::string SourceManager::generateSupport(std::set<v2::ASTNodeBase*> sel) {
  std::string ret;
  // suppress the warning
  // "typedef int bool;\n"
  ret += "#define true 1\n";
  ret += "#define false 0\n";
  // some code will config to have this variable.
  // Should not just define it randomly, but this is the best I can do to make it compile ...
  ret += "#define VERSION \"1\"\n";
  ret += "#define PACKAGE \"helium\"\n";
  // GNU Standard
  ret += "#define PACKAGE_NAME \"helium\"\n";
  ret += "#define PACKAGE_BUGREPORT \"xxx@xxx.com\"\n";
  ret += "#define PACKAGE_STRING \"helium 0.1\"\n";
  ret += "#define PACKAGE_TARNAME \"helium\"\n";
  ret += "#define PACKAGE_URL \"\"\n";
  ret += "#define PACKAGE_VERSION \"0.1\"\n";
  // unstandard primitive typedefs, such as u_char
  ret += "typedef unsigned char u_char;\n";
  ret += "typedef unsigned int u_int;\n";
  ret += "typedef unsigned char uchar;\n";
  ret += "typedef unsigned int uint;\n";
  ret += "typedef unsigned short u_short;\n";
  ret += "typedef unsigned short ushort;\n";

  // http://stackoverflow.com/questions/27459245/gcc-warning-with-std-c11-arg
  // https://www.gnu.org/software/libc/manual/html_mono/libc.html#Language-Features
  ret += "#define _POSIX_C_SOURCE 200809L\n";
  ret += "#define _DEFAULT_SOURCE\n";
  ret += "#define _GNU_SOURCE\n"; // this is everything haha

  // ret += "#include <stdbool.h>\n";
  // some must includes
  ret += "#include <stdio.h>\n";
  ret += "#include <stdlib.h>\n";
  ret += "#include <stdint.h>\n";
  // ret += "#include <string.h>\n";

  {
    std::set<std::string> headers = HeaderManager::Instance()->jsonGetHeaders();
    for (std::string s : headers) {
      ret += "#include <" + s + ">\n";
    }
  }

  sel = patchFunctionHeader(sel);
  // get the snippets

  // DEBUG
  // GlobalSnippetManager::Instance()->getManager()->dump(std::cout);
  
  std::set<v2::Snippet*> snippets;
  std::set<std::string> all_ids;
  for (auto *node : sel) {
    std::set<std::string> ids;
    if (node) {
      ids = node->getIdToResolve();
    }
    all_ids.insert(ids.begin(), ids.end());
    for (std::string id : ids) {
      std::vector<Snippet*> ss = v2::GlobalSnippetManager::Instance()->get(id);
      
      snippets.insert(ss.begin(), ss.end());
    }
  }
  
  
  std::cout << "[SourceManager] Got " << all_ids.size() << " IDs: ";
  for (std::string id : all_ids) {
    std::cout << id << " ";
  }
  std::cout << "\n";
  std::cout << "[SourceManager] queried " << snippets.size() << " snippts." << "\n";
  
  // get dependence
  std::set<v2::Snippet*> deps;
  for (auto *s : snippets) {
    // std::set<v2::Snippet*> dep = v2::GlobalSnippetManager::Instance()->getAllDep(s);
    std::set<v2::Snippet*> dep = s->getAllDeps();
    deps.insert(dep.begin(), dep.end());
  }
  snippets.insert(deps.begin(), deps.end());

  
  // remove non-outers
  snippets = v2::GlobalSnippetManager::Instance()->replaceNonOuters(snippets);

  
  // remove duplicate
  // for the same name and same type, only one should be retained.
  // This should be right before sortting
  struct SnippetComp {
    bool operator()(Snippet *s1,Snippet *s2) {
      std::set<std::string> key1 = s1->getKeys();
      std::set<std::string> key2 = s2->getKeys();
      std::string id1=s1->getSnippetName();
      std::string id2=s2->getSnippetName();
      for (std::string ss : key1) id1+=ss;
      for (std::string ss : key2) id2+=ss;
      // using keys, for anonemous enumerations which doesn't have a name, but have many fields
      return id1 < id2;
      // return s1->getName() + s1->getSnippetName() < s2->getName() + s2->getSnippetName();
    }
  };
  std::cout << "[SourceManager] " << snippets.size() << " Snippets BEFORE removing dup: ";
  for (Snippet *s : snippets) {
    std::cout << s->getName() << " "
              << s->getSnippetName() << "; ";
  }
  std::cout << "\n";
  std::set<Snippet*,SnippetComp> nodup;
  nodup.insert(snippets.begin(), snippets.end());
  snippets.clear();
  snippets.insert(nodup.begin(), nodup.end());
  std::cout << "[SourceManager] " << snippets.size() << " Snippets AFTER removing dup: ";
  for (Snippet *s : snippets) {
    std::cout << s->getName() << " "
              << s->getSnippetName() << "; ";
  }
  std::cout << "\n";
  // snippets = nodup;
  
  // sort the snippets
  std::vector<Snippet*> sorted_snippets = v2::GlobalSnippetManager::Instance()->sort(snippets);

  std::cout << "[SourceManager] " << sorted_snippets.size() << " Snippets used in main.h: ";
  for (Snippet *s : sorted_snippets) {
    std::cout << s->getName() << " ";
  }
  std::cout << "\n";

  std::vector<Snippet*> func_snippets;
  std::vector<Snippet*> type_snippets;
  std::vector<Snippet*> var_snippets;
  for (Snippet *s : sorted_snippets) {
    if (dynamic_cast<FunctionSnippet*>(s)) func_snippets.push_back(s);
    else if (dynamic_cast<TypedefSnippet*>(s)) type_snippets.push_back(s);
    else if (dynamic_cast<RecordSnippet*>(s)) type_snippets.push_back(s);
    else if (dynamic_cast<EnumSnippet*>(s)) type_snippets.push_back(s);
    else if (dynamic_cast<VarSnippet*>(s)) var_snippets.push_back(s);
  }


  // filter out functions used in main.c
  // std::cout << "Filtering ..." << "\n";
  // FIXME not only functions in main, but also: variable, structure
  // FIXME decl variables in main.h if it is in main.c, and got removed in snippets
  // FIXME for the same decl, although we should only use one, but if
  // one is extern int var, one is int var, we should use the int one.
  std::set<std::string> main_c_funcs;
  for (auto *node : sel) {
    // node->dump(std::cout);
    if (FunctionDecl *func = dynamic_cast<FunctionDecl*>(node)) {
      std::string name = func->getName();
      main_c_funcs.insert(name);
    }
  }
  // std::cout << "\n";
  // std::cout << "Main C:" << "\n";
  // for (std::string s : main_c_funcs) {
  //   std::cout << s << "\n";
  // }


  std::string type;
  std::string typedef_decl;
  std::string record_decl;
  for (Snippet *s : type_snippets) {
    type += "// " + s->toString() + "\n";
    type += s->getCode() + ";\n";
    if (TypedefSnippet *typedef_s = dynamic_cast<TypedefSnippet*>(s)) {
      typedef_decl += typedef_s->getDecl() + "\n";
    } else if (RecordSnippet *record_s = dynamic_cast<RecordSnippet*>(s)) {
      record_decl += record_s->getDecl() + "\n";
    // } else if (EnumSnippet *enum_s = dynamic_cast<EnumSnippet*>(s)) {
    //   record_decl += enum_s->getDecl() + "\n";
    }
  }

  std::string var;
  for (Snippet *s : var_snippets) {
    var += "// " + s->toString() + "\n";
    var += s->getCode() + ";\n";
  }
  // TRICK: the same function can be defined multiple times in the project it
  // is wired, the project might serve a purpose: gather many
  // interesting files, e.g. for education purpose.
  // I'm going to use the first one found, because if I output both, it is for sure compile error
  // - output the first one
  // - optional try all the combination
  // - print a fatal error
  std::set<std::string> func_trick;
  std::string func;
  std::string func_decl;
  std::string main_func_decl; // functions inside main
  for (Snippet *s : func_snippets) {
    std::string name = s->getName();
    // the declaration can be 
    if (main_c_funcs.count(name) == 0) {
      if (func_trick.count(name) == 1) {
        std::cerr << "[Helium Error] The function " << name << " is defined multiple times. Used the first one." << "\n";
      } else {
        func += "// " + s->toString() + "\n";
        func += s->getCode() + "\n";
        func_decl += dynamic_cast<FunctionSnippet*>(s)->getFuncDecl() + "\n";
        func_trick.insert(name);
      }
    } else {
      main_func_decl += dynamic_cast<FunctionSnippet*>(s)->getFuncDecl() + "\n";
    }
  }

  ret += GlobalSnippetManager::Instance()->dumpComment();
  
  ret += "// typedef_decl\n";
  ret += typedef_decl;
  ret += "// record_decl\n";
  ret += record_decl;
  
  ret += "// type\n";
  ret += type;
  ret += "// var\n";
  ret += var;

  ret += "// Main Func Decl\n";
  ret += main_func_decl;

  // I'm not able to put func_decl before var, becuase the var might define a type inner
  // The variable might hold a list of function pointers, that's the motivation to put decl before var.
  // This problem can only be solved when I generate the structure declaration.
  // This is an example:
  /**
   * node-threads-a-gogo
   static void barrier_wait(struct barrier *b);
   static struct barrier {
     int fd1[2];
     int fd2[2];
   } barriers[2];
   */
  ret += "// Func decl\n";
  // function declaration
  ret += func_decl;
  
  ret += "// func\n";
  ret += func;
  
  return ret;
}

std::string get_makefile() {
  std::string json_flag;
  {
    std::set<std::string> v = HeaderManager::Instance()->jsonGetFlags();
    for (std::string s : v) {
      json_flag += s + " ";
    }
  }
  
  std::string makefile;
  makefile += "CC:=clang\n";
  // makefile += "type $(CC) >/dev/null 2>&1 || { echo >&2 \"I require $(CC) but it's not installed.  Aborting.\"; exit 1; }\n";
  makefile += ".PHONY: all clean test\n";
  makefile = makefile + "a.out: main.c\n"
    + "\t$(CC) -g "
    // comment out because <unistd.h> will not include <optarg.h>
    + "-std=c11 "
    + "main.c "
    // gnulib should not be used:
    // 1. Debian can install it in the system header, so no longer need to clone
    // 2. helium-lib already has those needed headers, if installed correctly by instruction
    // + "-I$(HOME)/github/gnulib/lib " // gnulib headers
    + "-I/usr/include/x86_64-linux-gnu " // linux headers, stat.h
    + "-fprofile-arcs -ftest-coverage " // gcov coverage
    + json_flag
    + ""
    + "\n"
    + "clean:\n"
    + "\trm -rf *.out *.gcda *.gcno\n"
    + "test:\n"
    + "\tbash test.sh";
  return makefile;
}

void SourceManager::generate(std::set<v2::ASTNodeBase*> sel, fs::path dir) {
  std::string main_c = generateProgram(sel);
  std::string main_h = generateSupport(sel);
  std::string makefile = get_makefile();
  if (fs::exists(dir)) fs::remove_all(dir);
  fs::create_directories(dir);
  utils::write_file(dir / "main.c", main_c);
  utils::write_file(dir / "main.h", main_h);
  utils::write_file(dir / "Makefile", makefile);
}


void SourceManager::dump(std::ostream &os) {
  // maintained file to ASTs
  for (auto &m : File2ASTMap) {
    fs::path file = m.first;
    os << "file: " << file.string() << "\n";
    ASTContext *ast = m.second;
    // std::map<v2::ASTContext*, TokenVisitor*> AST2TokenVisitorMap;
    // std::map<v2::ASTContext*, Distributor*> AST2DistributorMap;
    os << "Distributor:" << "\n";
    Distributor *dist = AST2DistributorMap[ast];
    dist->dump(os);
  }
}
