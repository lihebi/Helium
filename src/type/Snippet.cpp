#include "helium/type/Snippet.h"
#include "helium/utils/fs_utils.h"

#include "helium/resolver/SnippetAction.h"
#include "helium/parser/HeaderManager.h"

#include "helium/utils/graph.h"
#include "helium/utils/StringUtils.h"


#include <rapidjson/istreamwrapper.h>

using namespace v2;

using std::set;
using std::vector;

GlobalSnippetManager *GlobalSnippetManager::instance = nullptr;

std::string read_file_for_code(fs::path file, SourceLocation begin, SourceLocation end) {
  int l1 = begin.getLine();
  int c1 = begin.getColumn();
  int l2 = end.getLine();
  int c2 = end.getColumn();
  
  std::ifstream is;
  is.open(file.string());
  int l=0;
  std::string ret;
  if (is.is_open()) {
    std::string line;
    while(getline(is, line)) {
      l++;
      if (l < l1 || l > l2) {
      } else if (l>l1 && l < l2) {
        ret += line + "\n";
      } else if (l==l1 && l == l2) {
        ret += line.substr(c1-1, c2+1 - c1);
      } else if (l==l2) {
        ret += line.substr(0, c2);
      } else if (l==l1) {
        ret += line.substr(c1-1) + "\n";
      } else {
        break;
      }
    }
    is.close();
  }
  return ret;
}

std::string read_file_for_code_until_semicolon(fs::path file, SourceLocation begin, SourceLocation end) {
  std::ifstream is;
  is.open(file.string());
  std::string ret;
  assert(is.is_open());
  int line=1,col=0;
  char c;
  while (is.get(c)) {
    if (c=='\n') {line++;col=0;}
    else col++;

    SourceLocation loc(line,col);

    if (loc < begin) ;
    else if (loc >= begin && loc <= end ) ret+=c;
    else {
      is.unget();
      break;
    }
  }
  while (is.get(c)) {
    if (c==';') break;
    ret += c;
  }
  is.close();
  return ret;
}

void v2::FunctionSnippet::readCode() {
  Code = read_file_for_code(File, Begin, End);
}
void v2::FunctionDeclSnippet::readCode() {
  Code = read_file_for_code_until_semicolon(File, Begin, End);
}
void v2::RecordSnippet::readCode() {
  Code = read_file_for_code(File, Begin, End);
}
void v2::RecordDeclSnippet::readCode() {
  Code = read_file_for_code_until_semicolon(File, Begin, End);
}
void v2::EnumSnippet::readCode() {
  Code = read_file_for_code(File, Begin, End);
}

/**
 * This performs the char-by-char read.
 * The variable does not stop until a semi-colon
 * I know this is a trick.
 */
void VarSnippet::readCode() {
  // this should read until a semi-colon
  Code = read_file_for_code_until_semicolon(File, Begin, End);
}

void TypedefSnippet::readCode() {
  Code = read_file_for_code_until_semicolon(File, Begin, End);
}

void v2::Snippet::dump(std::ostream &os) {
  os << Name;
}

void v2::Snippet::dumpVerbose(std::ostream &os) {
  rapidjson::Document doc;
  doc.SetObject();
  rapidjson::Value v = saveJson(doc.GetAllocator());
  rapidjson::StringBuffer sb;
  // rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
  rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
  v.Accept(writer);
  os << sb.GetString() << "\n";
}


std::string FunctionSnippet::getFuncDecl() {
  // it should be
  std::string code = getCode();
  std::string ret = code.substr(0, code.find('{'));
  utils::trim(ret);
  if (ret.find(';') == std::string::npos) {
    ret += ";";
  } else {
    std::string first, last;
    first = ret.substr(0, ret.find('('));
    last = ret.substr(ret.find(')')+1);
    std::string params;
    while (last.find(';') != std::string::npos) {
      int pos = last.find(';');
      params += last.substr(0, pos) + ",";
      last = last.substr(pos+1);
    }
    params.pop_back();
    ret = first + "(" + params + ");";
  }
  // add extern because inline function and inline declaration would
  // make undefined reference.  Note that this also applies to inline
  // function (which is the purpose to add this)
  // ret = "extern " + ret;
  //
  // UPDATE the extern method does not work because extern cannot be
  // combined with static So I'm going the second way: remove inline
  // when declare a function
  if (ret.find("inline") != std::string::npos) {
    utils::replace(ret, "inline", "");
  }
  return ret;
}


std::string TypedefSnippet::getDecl() {
  std::string code = getCode();
  if (code.find('{') == std::string::npos) {
    return code + ";";
  }
  assert(code.find('}') != std::string::npos);
  std::string ret;
  ret = code.substr(0, code.find_first_of('{'));
  {
    // if this is "typedef struct", return
    std::string tmp=ret;
    utils::trim(tmp);
    if (utils::split(tmp).size() == 2) return "";
  }
  ret += code.substr(code.find_last_of('}')+1);
  ret += ";";
  return ret;
}


std::string RecordSnippet::getDecl() {
  std::string code = getCode();
  std::string ret;
  if (code.find('{') == std::string::npos) return "";

  assert(code.find('}') != std::string::npos);
  ret = code.substr(0, code.find_first_of('{'));
  utils::trim(ret);
  std::vector<std::string> v = utils::split(ret);
  if (v.size() != 2) return "";
  ret += ";";
  return ret;
}

















void SnippetManager::createOuters() {
  // 1. enclosing
  // - go through all snippets, split them by files
  std::map<std::string, std::vector<Snippet*> > File2SnippetsMap;
  for (Snippet *s : Snippets) {
    File2SnippetsMap[s->getFile()].push_back(s);
  }
  // - order them based on begin loc
  for (auto &m : File2SnippetsMap) {
    std::vector<Snippet*> v = m.second;
    std::sort(v.begin(), v.end(), [](Snippet *lhs, Snippet *rhs) {
        SourceLocation loc1_begin = lhs->getBeginLoc();
        SourceLocation loc1_end = lhs->getEndLoc();
        SourceLocation loc2_begin = rhs->getBeginLoc();
        SourceLocation loc2_end = rhs->getEndLoc();
        if (loc1_begin < loc2_begin) return true;
        else if (loc1_begin == loc2_begin) return loc1_end > loc2_end;
        else return false;
      });
    // - for each, go through and find until the begin loc of the next is larget then its end, record that
    for (int i=0;i<v.size();i++) {
      for (int j=i+1;j<v.size();j++) {
        Snippet *s1 = v[i];
        Snippet *s2 = v[j];
        SourceLocation loc1_begin = s1->getBeginLoc();
        SourceLocation loc1_end = s1->getEndLoc();
        SourceLocation loc2_begin = s2->getBeginLoc();
        SourceLocation loc2_end = s2->getEndLoc();
        if (loc2_begin < loc1_end) {
          // Outers[s2].insert(s1);
          s2->addOuter(s1);
        }
      }
    }
  }
}

void SnippetManager::createDeps() {
  // 2. dep
  // - for each, get a list of dependence key
  for (Snippet *s : Snippets) {
    s->getCode();
    std::string name = s->getName();
    set<std::string> ids = extract_id_to_resolve(s->getCode());
    // do not query its name itself
    ids.erase(name);
    for (auto &id : ids) {
      // - query key map and create the dep
      s->addDep(get(id));
    }
    // remove itself
    s->removeDep(s);
  }
}



void SnippetManager::dumpVerbose(std::ostream &os) {
  os << "== Total " << Snippets.size() << " snippets\n";
  for (Snippet *s : Snippets) {
    os << "\t";
    // s->save(os);
    os << s->getCode();
    os << "\n";
  }
}

void SnippetManager::dump(std::ostream &os) {
  os << "[SnippetManager] " << Snippets.size() << " snippets, "
     << KeyMap.size() << " KeyMap entries." << "\n";
  std::set<Snippet*> func_s,typedef_s,enum_s,var_s,record_s,func_decl_s,record_decl_s;
  for (Snippet *s : Snippets) {
    if (dynamic_cast<FunctionSnippet*>(s)) {func_s.insert(s);}
    else if (dynamic_cast<TypedefSnippet*>(s)) {typedef_s.insert(s);}
    else if (dynamic_cast<EnumSnippet*>(s)) {enum_s.insert(s);}
    else if (dynamic_cast<VarSnippet*>(s)) {var_s.insert(s);}
    else if (dynamic_cast<RecordSnippet*>(s)) {record_s.insert(s);}
    else if (dynamic_cast<RecordDeclSnippet*>(s)) {record_decl_s.insert(s);}
    else if (dynamic_cast<FunctionDeclSnippet*>(s)) {func_decl_s.insert(s);}
  }
  // type of snippet
  os << "[SnipperManager] " << func_s.size() << " Functions: ";
  for (auto *s : func_s) {
    s->dump(os); os << " ";
  }
  os << "\n";
  os << "[SnipperManager] " << record_s.size() << " Records: ";
  for (auto *s : record_s) {
    s->dump(os); os << " ";
  }
  os << "\n";
  os << "[SnipperManager] " << typedef_s.size() << " Typedef: ";
  for (auto *s : typedef_s) {
    s->dump(os); os << " ";
  }
  os << "\n";
  os << "[SnipperManager] " << enum_s.size() << " Enum: ";
  for (auto *s : enum_s) {
    s->dump(os); os << " ";
  }
  os << "\n";
  os << "[SnipperManager] " << var_s.size() << " Var: ";
  for (auto *s : var_s) {
    s->dump(os); os << " ";
  }
  os << "\n";
  os << "[SnipperManager] " << func_decl_s.size() << " FuncDecl: ";
  for (auto *s : func_decl_s) {
    s->dump(os); os << " ";
  }
  os << "\n";
  os << "[SnipperManager] " << record_decl_s.size() << " RecordDecl: ";
  for (auto *s : record_decl_s) {
    s->dump(os); os << " ";
  }
  os << "\n";
}

std::string SnippetManager::dumpComment() {
  // sorted snippet
  // file dep
  // std::map<std::string, std::set<std::string> > FileDep;
  std::string ret;
  // file topo sort result, aka FileV
  ret += "// FileV: \n";
  for (auto s : FileV) {
    ret += "// \t" + s + "\n";
  }
  return ret;
}


std::set<v2::Snippet*> v2::Snippet::getAllDeps() {
  std::set<Snippet*> worklist;
  std::set<Snippet*> done;
  std::set<Snippet*> ret;
  worklist.insert(this);
  while (!worklist.empty()) {
    Snippet *s = *worklist.begin();
    worklist.erase(s);
    if (done.count(s) == 1) continue;
    std::set<Snippet*> dep = s->getDeps();
    worklist.insert(dep.begin(), dep.end());
    ret.insert(dep.begin(), dep.end());
    done.insert(s);
  }
  ret.erase(this);
  return ret;
}

std::set<v2::Snippet*> v2::SnippetManager::getAllDeps(std::set<v2::Snippet*> snippets) {
  std::set<v2::Snippet*> deps;
  for (auto *s : snippets) {
    // std::set<v2::Snippet*> dep = v2::GlobalSnippetManager::Instance()->getAllDep(s);
    std::set<v2::Snippet*> dep = s->getAllDeps();
    deps.insert(dep.begin(), dep.end());
  }
  snippets.insert(deps.begin(), deps.end());
  return snippets;
}

std::set<v2::Snippet*> SnippetManager::replaceNonOuters(std::set<Snippet*> ss) {
  std::set<Snippet*> ret;
  std::vector<Snippet*> worklist(ss.begin(), ss.end());
  std::set<Snippet*> done;
  while (!worklist.empty()) {
    Snippet *s = worklist.back();
    worklist.pop_back();
    done.insert(s);
    std::set<Snippet*> outers = s->getOuters();
    if (outers.empty()) ret.insert(s);
    for (Snippet *o : outers) {
      if (done.count(o) == 0) worklist.push_back(o);
    }
  }
  return ret;
}


void SnippetManager::traverseDir(fs::path dir) {
  fs::recursive_directory_iterator it(dir), eod;
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (p.extension() == ".c" || p.extension() == ".h") {
      std::vector<v2::Snippet*> snippets = createSnippets(p);
      add(snippets);
    }
  }
  process();
}



/**
 * Today I want to try a different approach.
 * 1. use the snippet got, infer the header dependence
 * 2. use that header dependence to sort the files
 */
std::vector<v2::Snippet*> SnippetManager::sort(std::set<Snippet*> snippets) {
  // DONE ELSEWHERE get header dependence
  // DONE ELSEWHERE sort within file
  // sort across file
  std::vector<v2::Snippet*> ret;
  // SnippetV is already sorted
  for (v2::Snippet *s : SnippetV) {
    if (snippets.count(s) == 1) ret.push_back(s);
  }
  return ret;
}

/**
 * Input: all snippets
 * Output:
 * - FileV: A vector of sorted files
 * - SnippetV
 * - FileMap
 */
void SnippetManager::sortFiles() {
  // std::cout << "Sorting files .." << "\n";
  // create File2SnippetMap
  FileMap.clear();
  SnippetV.clear();
  // FileDep.clear();
  // std::map<std::string, std::set<Snippet*> > FileMap;
  std::set<std::string> all_files;
  for (Snippet *s : Snippets) {
    FileMap[s->getFile()].insert(s);
    all_files.insert(s->getFile());
  }
  // HeaderManager::Instance()->dumpDeps(std::cout);

  
  std::vector<std::string> sorted = HeaderManager::Instance()->getSortedHeaders();
  std::vector<std::string> newsorted;
  for (std::string s : sorted) {
    if (all_files.count(s) == 1) {
      newsorted.push_back(s);
    }
  }
  // now add the others at the bottom This is because the header
  // manager will only sort those that are in dependency. For example,
  // If there're only one file, it is not in the result
  std::set<std::string> done(newsorted.begin(), newsorted.end());
  for (std::string s : all_files) {
    if (done.count(s) == 0) {
      done.insert(s);
      newsorted.push_back(s);
    }
  }
  FileV = newsorted;

  // std::cout << dumpComment() << "\n";
  for (std::string file : newsorted) {
    std::set<Snippet*> snippets = FileMap[file];
    std::vector<Snippet*> v(snippets.begin(), snippets.end());
    std::sort(v.begin(), v.end(), [](Snippet *lhs, Snippet *rhs){
        SourceLocation loc1 = lhs->getBeginLoc();
        SourceLocation loc2 = rhs->getBeginLoc();
        return loc1 < loc2;
      });
    SnippetV.insert(SnippetV.end(), v.begin(), v.end());
  }
  // snippet V size
  // std::cout << "SnippetV size: " << SnippetV.size() << "\n";
}


/**
 * Scheme
[
    {
        "name": "foo",
        "ID": 0,
        "kind": "Record",
        "file": "/path/to/file",
        "begin": {
            "line": 8,
            "col": 10
        },
        "end": {
            "line": 8,
            "col": 10
        },
        "deps": [2,5,8],
        "outers": [3,8]
    }
]

 */

/**
 * push back
 */
rapidjson::Value v2::Snippet::saveJson(rapidjson::Document::AllocatorType &allocator) {
  // rapidjson::Document::AllocatorType &allocator = document.GetAllocator();
  rapidjson::Value array(rapidjson::kObjectType);
  rapidjson::Value obj(rapidjson::kObjectType);
  rapidjson::Value name_str;
  name_str.SetString(Name.c_str(), allocator);
  obj.AddMember("name", name_str, allocator);
  obj.AddMember("id", ID, allocator);
  rapidjson::Value file_str;
  file_str.SetString(File.c_str(), allocator);
  obj.AddMember("file", file_str, allocator);
  // begin & end
  rapidjson::Value begin(rapidjson::kObjectType);
  rapidjson::Value end(rapidjson::kObjectType);
  begin.AddMember("line", Begin.getLine(), allocator);
  begin.AddMember("col", Begin.getColumn(), allocator);
  end.AddMember("line", End.getLine(), allocator);
  end.AddMember("col", End.getColumn(), allocator);
  obj.AddMember("begin", begin, allocator);
  obj.AddMember("end", end, allocator);

  // // dep & outer
  rapidjson::Value deps(rapidjson::kArrayType);
  rapidjson::Value outers(rapidjson::kArrayType);
  for (Snippet *dep : Deps) {
    deps.PushBack(dep->getId(), allocator);
  }
  for (Snippet *outer : Outers) {
    outers.PushBack(outer->getId(), allocator);
  }
  obj.AddMember("deps", deps, allocator);
  obj.AddMember("outers", outers, allocator);
  return obj;
  // document.PushBack(obj, allocator);
 }

void v2::Snippet::loadJson(rapidjson::Value &obj) {
  assert(obj.IsObject());
  Name = obj["name"].GetString();
  ID = obj["id"].GetInt();
  File = obj["file"].GetString();
  Begin = {obj["begin"]["line"].GetInt(),
           obj["begin"]["col"].GetInt()};
  End = {obj["end"]["line"].GetInt(),
           obj["end"]["col"].GetInt()};
  // deps and outers have to be add later
}
rapidjson::Value v2::EnumSnippet::saveJson(rapidjson::Document::AllocatorType &allocator) {
  rapidjson::Value v = Snippet::saveJson(allocator);
  v.AddMember("kind", "EnumSnippet", allocator);
  rapidjson::Value fields(rapidjson::kArrayType);
  for (std::string f : Fields) {
    rapidjson::Value f_s;
    f_s.SetString(f.c_str(), allocator);
    fields.PushBack(f_s, allocator);
  }
  v.AddMember("fields", fields, allocator);
  return v;
}
void v2::EnumSnippet::loadJson(rapidjson::Value &v) {
  Snippet::loadJson(v);
  for (rapidjson::Value &field : v["fields"].GetArray()) {
    Fields.push_back(field.GetString());
  }
}


// rapidjson::Value FunctionSnippet::saveJson(rapidjson::Document::AllocatorType &allocator) {
//   rapidjson::Value v = Snippet::saveJson(allocator);
//   v.AddMember("kind", "FunctionSnippet", allocator);
//   return v;
// }


void SnippetManager::saveJson(fs::path p) {
  rapidjson::Document document;
  rapidjson::Document::AllocatorType &allocator = document.GetAllocator();
  document.SetArray();
  assert(document.IsArray());
  for (Snippet *s : Snippets) {
    rapidjson::Value obj = s->saveJson(allocator);
    document.PushBack(obj, allocator);
  }

  rapidjson::StringBuffer sb;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
  document.Accept(writer);
  // sb.GetString();

  utils::write_file(p, sb.GetString());
  std::cout << "[SnippetManager] Saved to " << p << "\n";
}
void SnippetManager::loadJson(fs::path p) {
  rapidjson::Document document;
  std::ifstream ifs(p.string());
  rapidjson::IStreamWrapper isw(ifs);
  document.ParseStream(isw);
  assert(document.IsArray());
  // load snippets
  for (auto &v : document.GetArray()) {
    Snippet *s = nullptr;
    std::string kind = v["kind"].GetString();
    if (kind == "FunctionSnippet") s = new FunctionSnippet();
    else if (kind == "VarSnippet") s = new VarSnippet();
    else if (kind == "TypedefSnippet") s = new TypedefSnippet();
    else if (kind == "RecordSnippet") s = new RecordSnippet();
    else if (kind == "EnumSnippet") s = new EnumSnippet();
    else if (kind == "FunctionDeclSnippet") s = new FunctionDeclSnippet();
    else if (kind == "RecordDeclSnippet") s = new RecordDeclSnippet();
    else {assert(false);}

    s->loadJson(v);
    Snippets.push_back(s);
  }
  // load deps and outers
  for (auto &v : document.GetArray()) {
    int id = v["id"].GetInt();
    assert(id < Snippets.size());
    for (auto &dep : v["deps"].GetArray()) {
      int depId = dep.GetInt();
      assert(depId < Snippets.size());
      Snippets[id]->addDep(Snippets[depId]);
    }
    for (auto &outer : v["outers"].GetArray()) {
      int outerId = outer.GetInt();
      assert(outerId < Snippets.size());
      Snippets[id]->addOuter(Snippets[outerId]);
    }
  }
}



bool SnippetManager::checkValid(std::string &reason) {
  for (auto &m : KeyMap) {
    std::string key = m.first;
    std::vector<Snippet*> snippets = m.second;
    // if two functions, report
    bool func=false;
    for (Snippet *s : snippets) {
      if (dynamic_cast<FunctionSnippet*>(s)) {
        if (func) {
          reason = "function " + key + " is declared multiple times.";
          return false;
        } else {
          func = true;
        }
      }
    }
  }
  return true;
}
