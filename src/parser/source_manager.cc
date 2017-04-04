#include "helium/parser/source_manager.h"

#include "helium/parser/parser.h"
#include "helium/utils/string_utils.h"

#include "helium/utils/rand_utils.h"
#include "helium/parser/GrammarPatcher.h"

#include "helium/resolver/SnippetV2.h"

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
 */
fs::path SourceManager::matchFile(fs::path file) {
  fs::path ret;
  fs::path remaining;
  for (auto m : File2ASTMap) {
    fs::path f = m.first;
    // compare filename and move to parent
    // see how many times it is compared
    fs::path relf = fs::relative(f, cppfolder);
    // file in sel.txt
    fs::path file1 = file;
    // our records
    fs::path file2 = relf;
    while (file1.filename() == file2.filename() && !file1.empty()) {
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
      ret = relf;
    }
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
  std::set<v2::ASTNodeBase*> ret = sel;
  for (v2::ASTNodeBase *node : sel) {
    if (use2def.count(node) == 1) {
      std::set<ASTNodeBase*> tmp = use2def[node];
      ret.insert(tmp.begin(), tmp.end());
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
    os << "# " << file.string() << "\n";
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
 * for the selection, get the function node and put it into selection.
 * This includes:
 * - return node
 * - name node
 * - param node
 * - compount stmt
 * - comp token
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

std::string SourceManager::generateProgram(std::set<v2::ASTNodeBase*> sel) {
  // analyze distribution
  // std::map<v2::ASTContext*, Distributor*> AST2DistributorMap;
  sel = patchFunctionHeader(sel);
  
  int func_ct = 0;
  for (auto &m : AST2DistributorMap) {
    // ASTContext *ast = m.first;
    Distributor *dist = m.second;
    func_ct += dist->getDistFunc(sel);
  }
  if (func_ct == 0) return "";
  else if (func_ct == 1) {
    // only one function
    // - we don't include function headers.
    // - generate main function and put body inside function
    std::string body;
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
    std::string ret;
    ret += "#include <stdio.h>\n";
    ret += "#include <stdlib.h>\n";
    ret += "#include <string.h>\n";
    ret += "#include \"main.h\"\n";
    ret += "int main(int argc, char *argv[]) {\n";
    ret += body;
    ret += "  return 0;\n";
    ret += "}\n";
    return ret;
  } else {
    // multiple functions
    // - we always include function headers.
    // - put function call sites in main function
    std::string body;
    sel = patchFunctionHeader(sel);
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
    std::string ret;
    
    ret += "#include <stdio.h>\n";
    ret += "#include <stdlib.h>\n";
    ret += "#include <string.h>\n";
    ret += "#include \"main.h\"\n";

    ret += body;

    ret += "int main(int argc, char *argv[]) {\n";
    // TODO call to those functions
    ret += "  return 0;\n";
    ret += "}\n";
    return ret;
  }
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

  ret += "#include <stdbool.h>\n";
  ret += "#include <stdio.h>\n";
  ret += "#include <stdlib.h>\n";
  ret += "#include <string.h>\n";
  ret += "#include <getopt.h>\n";

  sel = patchFunctionHeader(sel);
  // get the snippets
  std::set<v2::Snippet*> snippets;
  for (auto *node : sel) {
    std::set<std::string> ids = node->getIdToResolve();
    for (std::string id : ids) {
      std::vector<Snippet*> ss = GlobalSnippetManager::Instance()->getManager()->get(id);
      snippets.insert(ss.begin(), ss.end());
    }
  }
  // sort the snippets
  std::vector<Snippet*> sorted_snippets = GlobalSnippetManager::Instance()->getManager()->sort(snippets);

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

  std::string type;
  for (Snippet *s : type_snippets) {
    type += s->getCode() + ";\n";
  }
  std::string var;
  for (Snippet *s : var_snippets) {
    var += s->getCode() + "\n";
  }
  std::string func;
  for (Snippet *s : func_snippets) {
    func += s->getCode() + "\n";
  }
  ret += type;
  ret += var;
  ret += func;
  
  return ret;
}

std::string get_makefile() {
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
