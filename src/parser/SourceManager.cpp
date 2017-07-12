#include "helium/parser/SourceManager.h"

#include "helium/parser/Parser.h"
#include "helium/utils/StringUtils.h"

#include "helium/utils/RandUtils.h"
#include "helium/parser/GrammarPatcher.h"

#include "helium/type/Snippet.h"

#include "helium/type/IOHelper.h"

#include <regex>



using std::string;
using std::vector;
using std::set;
using std::map;
using std::pair;



SourceManager::SourceManager(fs::path cppfolder) : cppfolder(cppfolder) {
  fs::recursive_directory_iterator it(cppfolder), eod;
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (p.extension() == ".c") {
      // std::cout << "[SourceManager] " << "parsing " << p.string() << "\n";
      Parser *parser = new Parser(p.string());
      ASTContext *ast = parser->getASTContext();
      ast->setSourceManager(this);
      // ASTs.push_back(ast);
      // files.push_back(p);
      File2ASTMap[p] = ast;
      AST2FileMap[ast] = p;
    }
  }

  // std::map<ASTContext*, TokenVisitor*> AST2TokenVisitorMap;
  // std::map<ASTContext*, Distributor*> AST2DistributorMap;

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
  //   std::map<ASTNodeBase*, int> levels = levelVisitor->getLevels();
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


std::set<ASTNodeBase*> SourceManager::defUse(std::set<ASTNodeBase*> sel) {
  std::map<ASTNodeBase*,std::set<ASTNodeBase*> > use2def;


  // for (auto &m : File2ASTMap) {
  //   ASTContext *ast = m.second;
  //   fs::path file = m.first;
  //   TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
  //   SymbolTableBuilder *symbolTableBuilder = new SymbolTableBuilder();
  //   unit->accept(symbolTableBuilder);
  //   std::map<ASTNodeBase*,std::set<ASTNodeBase*> > u2d = symbolTableBuilder->getUse2DefMap();
  //   use2def.insert(u2d.begin(), u2d.end());
  // }

  // getting only function level symbol table. I don't want to include
  // global variables, because they will be included through snippet
  for (auto &m : AST2DistributorMap) {
    Distributor *distributor = m.second;
    std::set<ASTNodeBase*> funcs = distributor->getFuncNodes();
    for (ASTNodeBase *func : funcs) {
      SymbolTableBuilder builder;
      func->accept(&builder);
      std::map<ASTNodeBase*,std::set<ASTNodeBase*> > u2d = builder.getUse2DefMap();
      use2def.insert(u2d.begin(), u2d.end());
      // std::string name = dynamic_cast<FunctionDecl*>(func)->getName();
      // if (name == "sdscatfmt") {
      //   builder.dump(std::cout);
      // }
      std::map<ASTNodeBase*, SymbolTable> tables = builder.getPersistTables();
      // merging into central storage
      PersistTables.insert(tables.begin(), tables.end());
    }
  }
  
  // process sel
  // this needs to be recursive
  std::set<ASTNodeBase*> ret = sel;

  // output variable is tricky
  // i'm going to put at the end of the generated function
  // the output is all used variables
  // this includes all input variables, plus those uses that is not input
  OutputVarNodes.clear();

  std::vector<ASTNodeBase*> worklist(sel.begin(), sel.end());
  std::set<ASTNodeBase*> done;
  while (!worklist.empty()) {
    ASTNodeBase *node = worklist.back();
    worklist.pop_back();
    done.insert(node);
    if (use2def.count(node) == 1) {
      std::set<ASTNodeBase*> tmp = use2def[node];
      // output vars
      OutputVarNodes.insert(tmp.begin(), tmp.end());
      ret.insert(tmp.begin(), tmp.end());
      for (auto *n : tmp) {
        if (done.count(n) == 0) worklist.push_back(n);
      }
    }
  }

  // record input and output variable
  // input variable is easy: the ones in ret but not in sel
  InputVarNodes.clear();
  for (ASTNodeBase* node : ret) {
    if (sel.count(node) == 0) {
      InputVarNodes.insert(node);
    }
  }
  
  // for (ASTNodeBase *node : sel) {
  //   if (use2def.count(node) == 1) {
  //     std::set<ASTNodeBase*> tmp = use2def[node];
  //     ret.insert(tmp.begin(), tmp.end());
  //   }
  // }
  return ret;
}

std::string SourceManager::getTokenUUID(ASTNodeBase* node) {
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

fs::path SourceManager::getTokenFile(ASTNodeBase* node) {
  ASTContext *ast = node->getASTContext();
  std::string ret;
  if (AST2FileMap.count(ast) == 1) {
    return AST2FileMap[ast];
  }
  return fs::path("");
}
int SourceManager::getTokenId(ASTNodeBase* node) {
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


std::set<ASTNodeBase*> SourceManager::generateRandomSelection() {
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

std::set<ASTNodeBase*> get_rand(std::vector<ASTNodeBase*> nodes, int num) {
  std::set<ASTNodeBase*> ret;
  // return all nodes or simply empty?
  if (num > nodes.size()) return ret;
  std::set<int> idxes = utils::rand_ints(0, nodes.size(), num);
  assert(idxes.size() == num);
  for (int idx : idxes) {
    ret.insert(nodes[idx]);
  }
  return ret;
}

std::set<ASTNodeBase*> SourceManager::genRandSel(int num) {
  // TODO get all tokens only one time to get performance
  std::vector<ASTNodeBase*> nodes;
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
std::set<ASTNodeBase*> SourceManager::genRandSelFunc(int num) {
  std::vector<ASTNodeBase*> functions;
  for (auto &m : AST2DistributorMap) {
    Distributor *dist = m.second;
    std::set<ASTNodeBase*> funcs = dist->getFuncNodes();
    functions.insert(functions.end(), funcs.begin(), funcs.end());
  }
  std::vector<ASTNodeBase*> nodes;
  for (ASTNodeBase* func : functions) {
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
std::set<ASTNodeBase*> SourceManager::genRandSelSameFunc(int num) {
  // get a vector of vector of nodes, ordered by function
  std::vector<ASTNodeBase*> functions;
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


/**
 * Selection format:
 * [
 // the selection is a list of files
 {
 file: "/abs/path/to/src.c",
 sel: [
 // each file specify a list of selection
   {
   line: 8, // 1. select both line and column
   col: 9
   },
   {line: 20}, // 2. select only line
   {
   line: 10,
   col: 20
   }
 ]
 }
 * ]
 */

std::set<ASTNodeBase*> SourceManager::loadJsonSelection(fs::path sel_file) {
  map<string, set<pair<int, int> > > selection;
  if (fs::exists(sel_file)) {
    rapidjson::Document document;
    std::ifstream ifs(sel_file.string());
    rapidjson::IStreamWrapper isw(ifs);
    document.ParseStream(isw);
    assert(document.IsArray());
    // iterate through the list of files
    for (rapidjson::Value &field : document.GetArray()) {
      assert(field.IsObject());
      std::string file = field["file"].GetString();
      for (rapidjson::Value &sel : field["sel"].GetArray()) {
        assert(sel.IsObject());
        int line = -1, col = -1;
        if (sel.HasMember("line")) {
          line = sel["line"].GetInt();
        }
        if (sel.HasMember("col")) {
          col = sel["col"].GetInt();
        }
        // add line and col into selection
        selection[file].insert(std::make_pair(line, col));
      }
    }
  }
  // from the selection, get ASTNodes
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
      vector<ASTNodeBase*> tokens = tokenVisitor->getTokens();
      map<ASTNodeBase*,int> idmap = tokenVisitor->getIdMap();
      for (const pair<int,int> &p : sel.second) {
        int line = p.first;
        int column = p.second;
        // I know this is inefficient
        for (ASTNodeBase *token : tokens) {
          SourceLocation begin = token->getBeginLoc();
          SourceLocation end = token->getEndLoc();
          if (column == -1) {
            // only need to compare the line
            if (begin.getLine() <= line && line <= end.getLine()) {
              ret.insert(token);
            }
          } else {
            SourceLocation loc(line, column);
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
  }
  return ret;
}


std::set<ASTNodeBase*> SourceManager::loadSelection(fs::path sel_file) {
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
      vector<ASTNodeBase*> tokens = tokenVisitor->getTokens();
      map<ASTNodeBase*,int> idmap = tokenVisitor->getIdMap();
      for (const pair<int,int> &p : sel.second) {
        int line = p.first;
        int column = p.second;
        SourceLocation loc(line, column);
        // I know this is inefficient
        for (ASTNodeBase *token : tokens) {
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


void SourceManager::dumpJsonSelection(std::set<ASTNodeBase*> selection, std::ostream &os) {
  // dump to os
  // document: array of fileObjs
  rapidjson::Document doc;
  doc.SetArray();
  rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
  for (auto &m : AST2TokenVisitorMap) {
    ASTContext *ast = m.first;
    TokenVisitor *tokenVisitor = m.second;
    fs::path file = AST2FileMap[ast];
    // fileObj: one for a single file
    rapidjson::Value fileObj;
    fileObj.SetObject();
    // file attribute
    {
      rapidjson::Value str;
      str.SetString(file.string().c_str(), allocator);
      fileObj.AddMember("file", str, allocator);
    }
    // array of sel attribute 
    rapidjson::Value selArray;
    selArray.SetArray();
    vector<ASTNodeBase*> tokens = tokenVisitor->getTokens();
    for (ASTNodeBase *token : tokens) {
      if (selection.count(token) == 1) {
        SourceLocation loc = token->getBeginLoc();
        rapidjson::Value locObj;
        locObj.SetObject();
        locObj.AddMember("line", loc.getLine(), allocator);
        locObj.AddMember("col", loc.getColumn(), allocator);
        selArray.PushBack(locObj, allocator);
      }
    }
    fileObj.AddMember("sel", selArray, allocator);

    // add this fileObj to doc
    doc.PushBack(fileObj, allocator);
  }
  // save the doc to the os
  rapidjson::StringBuffer sb;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
  doc.Accept(writer);
  os << sb.GetString() << "\n";
}


void SourceManager::dumpSelection(std::set<ASTNodeBase*> selection, std::ostream &os) {
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

void SourceManager::dumpDist(std::set<ASTNodeBase*> sel, std::ostream &os) {
  // these are not interesting actually
  // These are how many the specific type of nodes ENCLOSING sel
  os << "[DumpDist] Size: " << sel.size() << "\n";
  os << "[DumpDist] File: " << getDistFile(sel) << "\n";
  os << "[DumpDist] Proc: " << getDistProc(sel) << "\n";
  os << "[DumpDist] If: " << getDistIf(sel) << "\n";
  os << "[DumpDist] Switch: " << getDistSwitch(sel) << "\n";
  os << "[DumpDist] Loop: " << getDistLoop(sel) << "\n";

  // these are more interesting
  // these are all nodes
  int if_ct=0;
  int switch_ct=0;
  int loop_ct=0;
  for (ASTNodeBase* node : sel) {
    if (dynamic_cast<IfStmt*>(node)) {
      if_ct++;
    } else if (dynamic_cast<SwitchStmt*>(node)) {
      switch_ct++;
    } else if (dynamic_cast<WhileStmt*>(node)
               || dynamic_cast<DoStmt*>(node)
               || dynamic_cast<ForStmt*>(node)) {
      loop_ct++;
    }
  }
  os << "[DumpDist] IfNode: " << if_ct << "\n";
  os << "[DumpDist] SwitchNode: " << switch_ct << "\n";
  os << "[DumpDist] LoopNode: " << loop_ct << "\n";
  // lets check for only the local nodes
}

/**
 * only leaf nodes in sel will be returned
 */
std::set<ASTNodeBase*> SourceManager::filterLeaf(std::set<ASTNodeBase*> sel) {
  std::set<ASTNodeBase*> ret;
  for (ASTNodeBase *node : sel) {
    if (node->isLeaf()) ret.insert(node);
  }
  return ret;
}

// #tok | #patch |
// #file | #per(file) | #proc | #per(proc) | #if | #per(if)
// #loop | #per(loop) | # switch | #per(switch)
// result
void SourceManager::analyzeDistribution(std::set<ASTNodeBase*> selection,
                                        std::set<ASTNodeBase*> patch,
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






std::set<ASTContext*> SourceManager::getASTByNodes(std::set<ASTNodeBase*> nodes) {
  std::set<ASTContext*> ret;
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
std::set<ASTNodeBase*> SourceManager::patchFunctionHeader(std::set<ASTNodeBase*> sel) {
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
    std::set<ASTContext*> asts = getASTByNodes(sel);
    std::set<ASTNodeBase*> funcs;
    for (ASTContext *ast : asts) {
      Distributor *dist = AST2DistributorMap[ast];
      std::set<ASTNodeBase*> nodes = dist->getDistFuncNodes(sel);
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

bool SourceManager::shouldPutIntoMain(std::set<ASTNodeBase*> sel) {
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

/**
 * return the last of the nodes
 */
ASTNodeBase *lastNode(std::set<ASTNodeBase*> nodes) {
  ASTNodeBase* ret = nullptr;
  for (auto *node : nodes) {
    if (!ret) ret=node;
    else {
      // should not be compound stmt, because that would make a wrong output position
      if (dynamic_cast<CompoundStmt*>(node)) continue;
      SourceLocation loc = node->getEndLoc();
      SourceLocation retloc = ret->getEndLoc();
      if (retloc < loc) {
        ret = node;
      }
    }
  }
  return ret;
}

std::string
SourceManager::generateProgram(std::set<ASTNodeBase*> sel) {
  // analyze distribution
  // std::map<ASTContext*, Distributor*> AST2DistributorMap;
  sel = patchFunctionHeader(sel);

  std::string ret;
  // ret += "#include <stdio.h>\n";
  // ret += "#include <stdlib.h>\n";
  // ret += "#include <string.h>\n";
  ret += "#include \"main.h\"\n";

  ret += FileIOHelper::GetIOCode();

  struct IOData {
    int output_var=0;
    int used_output_var=0;
    int input_var=0;
    int used_input_var=0;
  } iodata;
  
  if (shouldPutIntoMain(sel)) {
    std::string body;
    for (auto &m : File2ASTMap) {
      Generator *generator =  new Generator();
      generator->setSelection(sel);
      generator->adjustReturn(true);
      // generator->setSelection(sel);


      // set input position
      generator->setInputVarNodes(InputVarNodes);
      // set output var and position
      // 1. get the last of the selection
      ASTNodeBase *last = lastNode(sel);
      // 2. get the vars used in the sel => stored in OutputVarNodes
      // 3. get the alive ones at the last node of sel
      // FIXME make sure this exists
      SymbolTable symbol_table = PersistTables[last];
      // std::set<std::string> symbols = symbol_table.getNewlyAdded();
      // newly added is used to get the really used vars from the "input/output var nodes"
      std::set<std::string> usedvars;
      for (auto *node : sel) {
        std::set<std::string> vars = node->getUsedVars();
        usedvars.insert(vars.begin(), vars.end());
      }
      std::map<std::string, ASTNodeBase*> allsymbols = symbol_table.getAll();
      std::map<std::string, ASTNodeBase*> toinstrument;
      for (std::string var : usedvars) {
        if (allsymbols.count(var) == 1) {
          toinstrument[var] = allsymbols[var];
        }
      }
      // 4. instrument after the last of sel
      generator->setOutputInstrument(toinstrument, last, symbol_table);
      
      ASTContext *ast = m.second;
      TranslationUnitDecl *decl = ast->getTranslationUnitDecl();
      decl->accept(generator);
      std::string prog = generator->getProgram();
      body += prog;
      std::vector<int> iodata_raw = generator->getIOSummary();
      assert(iodata_raw.size() == 4);
      iodata.output_var += iodata_raw[0];
      iodata.used_output_var += iodata_raw[1];
      iodata.input_var += iodata_raw[2];
      iodata.used_input_var += iodata_raw[3];
    }
    ret += "// Should into main\n";
    ret += "int main(int argc, char *argv[]) {\n";
    ret += FileIOHelper::GetFilePointersInit();
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
    ret += FileIOHelper::GetFilePointersInit();
    // TODO call to those functions
    ret += "  return 0;\n";
    ret += "}\n";
  }
  // at the end, I'm going to put some comments about IOData
  ret += "// IOData.output_var = " + std::to_string(iodata.output_var) + "\n";
  ret += "// IOData.used_output_var = " + std::to_string(iodata.used_output_var) + "\n";
  ret += "// IOData.input_var = " + std::to_string(iodata.input_var) + "\n";
  ret += "// IOData.used_input_var = " + std::to_string(iodata.used_input_var) + "\n";


  // output to stdout as log
  std::cout << "[SourceManager] IOData (I|used/all/O|used/all) "
            << iodata.used_input_var << " / " << iodata.input_var << " | "
            << iodata.used_output_var << " / " << iodata.output_var << "\n";
  std::cerr << "[SourceManager] IOData (I|used/all/O|used/all) "
            << iodata.used_input_var << " / " << iodata.input_var << " | "
            << iodata.used_output_var << " / " << iodata.output_var << "\n";
  return ret;
}



static std::set<Snippet*>
get_snippets(std::set<ASTNodeBase*> sel,
             SnippetManager *snip_man) {
  std::set<Snippet*> ret;
  // get keys
  std::set<std::string> keys;
  for (auto *node : sel) {
    assert(node);
    std::set<std::string> v = node->getIdToResolve();
    keys.insert(v.begin(), v.end());
  }
  // query snippets
  for (std::string key : keys) {
    std::vector<Snippet*> ss = snip_man->getAll(key);
    ret.insert(ss.begin(), ss.end());
  }
  // get dep and outer
  std::set<Snippet*> back;
  // FIXME infinite loop
  while (back != ret) {
    back = ret;
    // std::cout << "[SourceManager] all dep and outer infinite loop ..\n";
    ret = snip_man->getAllDeps(ret);
    ret = snip_man->replaceNonOuters(ret);
  }
  return ret;
}

/**
 * FIXME this is buggy, the comp functor is hard to get right, i want
 * to remove duplicate for some snippet (function), but want to keep
 * for others (functiondecl)
 */
static std::set<Snippet*> remove_dup(std::set<Snippet*> snippets) {
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

      // FIXME but I want duplicate for certain snippets, like decl
      // (HEBI: FIXME)
      // if (id1 == id2 &&
      //     (id1 == "FunctionDeclSnippet" || id1 == "RecordDeclSnippet")) {
      //   id2+="2";
      // }
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
  return snippets;
}



// DEPRECATED
std::string get_snippet_code_separate(vector<Snippet*> sorted_snippets, std::set<std::string> main_c_funcs) {
  std::string ret;
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

static std::string get_snippet_code_sort(vector<Snippet*> sorted_snippets, set<string> main_c_funcs) {
  std::string ret;
  std::set<FunctionSnippet*> main_funcs;
  for (Snippet *s : sorted_snippets) {
    if (FunctionSnippet *func = dynamic_cast<FunctionSnippet*>(s)) {
      std::string name = s->getName();
      if (main_c_funcs.count(name) == 0) {
        ret += "// " + s->toString() + "\n";
        ret += s->getCode() + "\n";
      } else {
        main_funcs.insert(func);
        ret += "// replacing define with decl: \n";
        ret += func->getFuncDecl() + "\n";
      }
    } else {
      ret += "// " + s->toString() + "\n";
      // well, always add a ; does not hurt ..
      ret += s->getCode() + ";\n";
    }
  }
  // now get the main func decl
  // TODO to reserve the order, I actually should output the function
  // there instead of in main
  // ret += "// Main Func Decl\n";
  // for (auto *f : main_funcs) {
  //   ret += f->getFuncDecl() + "\n";
  // }
  return ret;
}

static std::string get_common_header() {
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

  // some must includes
  ret += "#include <stdio.h>\n";
  ret += "#include <stdlib.h>\n";
  // must include this because the projects often typedef bool
  // themself, but i treat bool as a keyword when resolving snippets
  ret += "#include <stdbool.h>\n";
  ret += "#include <stdint.h>\n";
  // ret += "#include <string.h>\n";
  return ret;
}

static std::vector<std::string>
get_system_includes(IncludeManager *inc_manager,
                    LibraryManager *lib_manager) {
  std::vector<std::string> ret;
  std::set<std::string> system_incs = inc_manager->getSystemIncludes();
  for (std::string inc : system_incs) {
    Library *lib = lib_manager->findLibraryByInclude(inc);
    if (lib) {
      ret.push_back(inc);
    }
  }
  return ret;
}

std::string SourceManager::generateSupport(std::set<ASTNodeBase*> sel,
                                           SnippetManager *snip_man,
                                           IncludeManager *inc_man,
                                           LibraryManager *lib_man) {
  std::string ret;
  ret += get_common_header();
  for (std::string inc : get_system_includes(inc_man, lib_man)) {
    ret += "#include <" + inc + ">\n";
  }

  sel = patchFunctionHeader(sel);
  
  // get the snippets
  std::set<Snippet*> snippets = get_snippets(sel, snip_man);
  snippets = remove_dup(snippets);
  // sort the snippets
  std::vector<Snippet*> sorted_snippets = snip_man->sort(snippets);
  // filter out functions used in main.c
  std::set<std::string> main_c_funcs;
  for (auto *node : sel) {
    // node->dump(std::cout);
    if (FunctionDecl *func = dynamic_cast<FunctionDecl*>(node)) {
      std::string name = func->getName();
      main_c_funcs.insert(name);
    }
  }
  std::string snippet_code = get_snippet_code_sort(sorted_snippets, main_c_funcs);

  ret += snippet_code;
  return ret;
}

static std::string
get_system_flags(IncludeManager *inc_man,
                 LibraryManager *lib_man) {
  std::set<Library*> used_libs;
  std::set<std::string> system_incs = inc_man->getSystemIncludes();
  for (std::string inc : system_incs) {
    Library *lib = lib_man->findLibraryByInclude(inc);
    if (lib) {
      used_libs.insert(lib);
    }
  }
  // get flags
  std::string ret;
  for (Library *lib : used_libs) {
    if (lib->exists()) {
      ret += lib->getFlags() + " ";
    } else {
      // warning
    }
  }
  return ret;
}

std::string get_makefile(IncludeManager *inc_man, LibraryManager *lib_man) {
  std::string prefix;
  prefix += "CC:=clang\n";
  prefix += "SYSTEM_FLAGS=" + get_system_flags(inc_man, lib_man) + "\n";
  prefix += "COV_FLAGS=-fprofile-arcs -ftest-coverage\n";
  prefix += "\n";
  prefix += ".PHONY: all clean test\n";

  const char *raw = R"prefix(
a.out: main.c
	$(CC) -g -std=c11 main.c $(COV_FLAGS) $(SYSTEM_FLAGS)

clean:
	rm -rf *.out *.gcda *.gcno
test:
	bash test.sh
run:
	timeout 1 ./a.out 2>&1 >/dev/null
)prefix";

  std::string ret;
  ret += prefix + raw;
  return ret;
}

void SourceManager::generate(std::set<ASTNodeBase*> sel,
                             fs::path dir,
                             SnippetManager *snip_man,
                             IncludeManager *inc_man,
                             LibraryManager *lib_man) {
  std::string main_c = generateProgram(sel);
  std::string main_h = generateSupport(sel, snip_man, inc_man, lib_man);
  std::string makefile = get_makefile(inc_man, lib_man);
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
    // std::map<ASTContext*, TokenVisitor*> AST2TokenVisitorMap;
    // std::map<ASTContext*, Distributor*> AST2DistributorMap;
    os << "Distributor:" << "\n";
    Distributor *dist = AST2DistributorMap[ast];
    dist->dump(os);
  }
}
