#include "helium/resolver/SnippetV2.h"
#include "helium/utils/fs_utils.h"

#include "helium/resolver/SnippetAction.h"

#include "helium/resolver/resolver.h"
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
        ret += line.substr(c1-1, c1+1 - c1);
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
void v2::RecordSnippet::readCode() {
  Code = read_file_for_code(File, Begin, End);
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
        else if (loc1_begin == loc2_begin) return loc1_end < loc2_end;
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
          Outers[s2].insert(s1);
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
      vector<Snippet*> v = get(id);
      Deps[s].insert(v.begin(), v.end());
      Deps[s].erase(s);
    }
  }
}




/**
 * Disk
 */
void SnippetManager::saveSnippet(fs::path p) {
  std::ofstream os;
  os.open(p.string());
  assert(os.is_open());
  for (Snippet *s : Snippets) {
    s->save(os);
  }
}
void SnippetManager::loadSnippet(fs::path p) {
  assert(Snippets.empty());
  std::ifstream is;
  is.open(p.string());
  assert(is.is_open());
  std::string kind;
  Snippet *s = nullptr;
  while (is >> kind) {
    if (kind == "FunctionSnippet") s = new FunctionSnippet();
    else if (kind == "VarSnippet") s = new VarSnippet();
    else if (kind == "TypedefSnippet") s = new TypedefSnippet();
    else if (kind == "RecordSnippet") s = new RecordSnippet();
    else {
      assert(false);
    }
    s->load(is);
    int id = Snippets.size();
    s->setId(id);
    Snippets.push_back(s);
  }
}

void SnippetManager::saveDeps(fs::path p) {
  std::ofstream os;
  os.open(p.string());
  assert(os.is_open());
  for (auto &m : Deps) {
    os << m.first->getId() << " ";
    for (Snippet *s : m.second) {
      os << s->getId() << " ";
    }
    os << "\n";
  }
}

void SnippetManager::loadDeps(fs::path p) {
  std::ifstream is;
  is.open(p.string());
  assert(is.is_open());
  std::string line;
  while (getline(is, line)) {
    int from_id,to_id;
    std::istringstream ss(line);
    ss >> from_id;
    while (ss >> to_id) {
      // IdDeps[from_id].insert(to_id);
      Deps[Snippets[from_id]].insert(Snippets[to_id]);
    }
  }
}

void SnippetManager::saveOuters(fs::path p) {
  std::ofstream os;
  os.open(p.string());
  assert(os.is_open());
  for (auto &m : Outers) {
    os << m.first->getId() << " ";
    for (Snippet *s : m.second) {
      os << s->getId() << " ";
    }
    os << "\n";
  }
}

void SnippetManager::loadOuters(fs::path p) {
  std::ifstream is;
  is.open(p.string());
  assert(is.is_open());
  std::string line;
  while (getline(is, line)) {
    int from_id,to_id;
    std::istringstream ss(line);
    ss >> from_id;
    while (ss >> to_id) {
      // IdDeps[from_id].insert(to_id);
      Outers[Snippets[from_id]].insert(Snippets[to_id]);
    }
  }
}




void SnippetManager::dumpSnippetsVerbose(std::ostream &os) {
  os << "== Total " << Snippets.size() << " snippets\n";
  for (Snippet *s : Snippets) {
    os << "\t";
    s->save(os);
    os << s->getCode();
    os << "\n";
  }
}

void SnippetManager::dumpLight(std::ostream &os) {
  os << "Snippets: " << "\n";
  int func_ct = 0;
  int var_ct = 0;
  int record_ct = 0;
  int typedef_ct = 0;
  int enum_ct = 0;
  for (Snippet *s : Snippets) {
    if (dynamic_cast<FunctionSnippet*>(s)) func_ct++;
    else if (dynamic_cast<TypedefSnippet*>(s)) typedef_ct++;
    else if (dynamic_cast<EnumSnippet*>(s)) enum_ct++;
    else if (dynamic_cast<VarSnippet*>(s)) var_ct++;
    else if (dynamic_cast<RecordSnippet*>(s)) record_ct++;
  }
  // type of snippet
  os << "\t" << "Function: " << func_ct << "\n";
  os << "\t" << "Record: " << record_ct << "\n";
  os << "\t" << "Typedef: " << typedef_ct << "\n";
  os << "\t" << "Enum: " << enum_ct << "\n";
  os << "\t" << "Var: " << var_ct << "\n";

  int deps_ct = 0;
  for (auto &m : Deps) {
    deps_ct += m.second.size();
  }
  os << "Deps: " << deps_ct << "\n";

  int outers_ct = 0;
  for (auto &m : Outers) {
    outers_ct += m.second.size();
  }
  os << "Deps: " << outers_ct << "\n";
}

void SnippetManager::dump(std::ostream &os) {
  os << "== Total " << Snippets.size() << " snippets\n";
  for (Snippet *s : Snippets) {
    // s->dump(os);
    // os << "\n";
    // s->save(os);
    // os << s->getCode();
    os << "\t";
    s->dump(os);
    os << "\n";
  }
  os << "== Dependence:" << "\n";
  // std::map<Snippet*, std::set<Snippet*> > Deps;
  // std::map<Snippet*, std::set<Snippet*> > Outer;
  for (auto &m : Deps) {
    Snippet *from = m.first;
    std::set<Snippet*> to = m.second;
    os << "\t";
    from->dump(os);
    os << " ==> ";
    for (Snippet *s : to) {
      s->dump(os);
      os << ", ";
    }
    os << "\n";
  }
  os << "== Outers:" << "\n";
  for (auto &m : Outers) {
    Snippet *from = m.first;
    std::set<Snippet*> outers = m.second;
    os << "\t";
    from->dump(os);
    os << " ==> ";
    for (Snippet *s : outers) {
      s->dump(os);
      os << ", ";
    }
    os << "\n";
  }
}


std::set<v2::Snippet*> SnippetManager::getAllDep(Snippet *s) {
  std::set<Snippet*> worklist;
  std::set<Snippet*> done;
  std::set<Snippet*> ret;
  worklist.insert(s);
  while (!worklist.empty()) {
    Snippet *s = *worklist.begin();
    if (done.count(s) == 1) continue;
    worklist.erase(s);
    std::set<Snippet*> dep = getDep(s);
    worklist.insert(dep.begin(), dep.end());
    ret.insert(dep.begin(), dep.end());
    done.insert(s);
  }
  ret.erase(s);
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
