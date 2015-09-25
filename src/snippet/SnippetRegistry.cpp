#include "snippet/SnippetRegistry.hpp"

#include "snippet/FunctionSnippet.hpp"
#include "snippet/EnumSnippet.hpp"
#include "snippet/DefineSnippet.hpp"
#include "snippet/StructureSnippet.hpp"
#include "snippet/TypedefSnippet.hpp"
#include "snippet/UnionSnippet.hpp"
#include "snippet/VariableSnippet.hpp"

#include "resolver/Ctags.hpp"
#include "resolver/Resolver.hpp"
#include "util/FileUtil.hpp"
#include "util/StringUtil.hpp"

#include <iostream>

SnippetRegistry* SnippetRegistry::m_instance = 0;

/**************************
 ******* Look Up **********
 **************************/
std::set<Snippet*>
SnippetRegistry::LookUp(const std::string& name) {
  if (m_id_map.find(name) != m_id_map.end()) {
    return m_id_map[name];
  }
  return std::set<Snippet*>();
}
// look up by type
Snippet*
SnippetRegistry::LookUp(const std::string& name, char type) {
  if (m_id_map.find(name) != m_id_map.end()) {
    std::set<Snippet*> vs = m_id_map[name];
    for (auto it=vs.begin();it!=vs.end();it++) {
      if ((*it)->GetType() == type) {
        return *it;
      }
    }
  }
  return NULL;
}
// look up by types
std::set<Snippet*>
SnippetRegistry::LookUp(const std::string& name, const std::string& type) {
  if (m_id_map.find(name) != m_id_map.end()) {
    std::set<Snippet*> vs = m_id_map[name];
    for (auto it=vs.begin();it!=vs.end();it++) {
      if (type.find((*it)->GetType()) == -1) {
        vs.erase(it);
      }
    }
    return vs;
  }
  return std::set<Snippet*>();
}

/**************************
 ******* Dependence **********
 **************************/

std::set<Snippet*>
SnippetRegistry::GetDependence(Snippet* snippet) {
  if (m_dependence_map.find(snippet) != m_dependence_map.end()) {
    return m_dependence_map[snippet];
  } else {
    return std::set<Snippet*>();
  }
}
// recursively get dependence
std::set<Snippet*>
SnippetRegistry::GetAllDependence(Snippet* snippet) {
  std::set<Snippet*> snippets;
  snippets.insert(snippet);
  return GetAllDependence(snippets);
}

/*
 * Get all snippets dependence
 * return including the params
 */
std::set<Snippet*>
SnippetRegistry::GetAllDependence(std::set<Snippet*> snippets) {
  std::set<Snippet*> all_snippets;
  std::set<Snippet*> to_resolve;
  for (auto it=snippets.begin();it!=snippets.end();it++) {
     all_snippets.insert(*it);
     to_resolve.insert(*it);
  }
  while (!to_resolve.empty()) {
    Snippet *tmp = *(to_resolve.begin());
    to_resolve.erase(tmp);
    all_snippets.insert(tmp);
    std::set<Snippet*> dep = GetDependence(tmp);
    for (auto it=dep.begin();it!=dep.end();it++) {
      if (all_snippets.find(*it) == all_snippets.end()) {
        // not found, new snippet
        to_resolve.insert(*it);
      }
    }
  }
  return all_snippets;
}

/**************************
 ********** Add ***********
 **************************/

Snippet*
SnippetRegistry::Add(const CtagsEntry& ce) {
  std::cout << "[SnippetRegistry::Add]" << std::endl;
  Snippet *s = createSnippet(ce);
  if (!s) return NULL;
  // lookup to remove duplicate
  std::set<std::string> keywords = s->GetKeywords();
  for (auto it=keywords.begin();it!=keywords.end();it++) {
    // std::cout << *it << std::endl;
    // std::cout << ce.GetType() << std::endl;
    // Snippet *s_tmp = LookUp(*it, ce.GetType());
    // the ce.GetType() maybe 't', but actually the snippet is a structure.
    // so use the global one?
    std::set<Snippet*> snippets = LookUp(*it);
    if (!snippets.empty()) {
      return *snippets.begin();
    }
    // if (s_tmp) return s_tmp;
  }
  // insert
  add(s);
  resolveDependence(s);
  return s;
}

void
SnippetRegistry::resolveDependence(Snippet *s) {
  // std::cout << "[SnippetRegistry::resolveDependence]" << std::endl;
  std::set<std::string> ss = Resolver::ExtractToResolve(s->GetCode());
  // std::cout << "[SnippetRegistry::resolveDependence] size of to resolve: " << ss.size() << std::endl;
  for (auto it=ss.begin();it!=ss.end();it++) {
    // FIXME if it is enum member, this can never hit ...
    if (!LookUp(*it).empty()) {
      // already resolved. Just add dependence
      addDependence(s, LookUp(*it));
    } else {
      // resolve by ctags
      std::vector<CtagsEntry> vc = Ctags::Instance()->Parse(*it);
      if (!vc.empty()) {
        for (auto it2=vc.begin();it2!=vc.end();it2++) {
          Snippet *snew = createSnippet(*it2);
          if (snew) {
            add(snew);
            addDependence(s, snew);
            resolveDependence(snew);
          }
        }
      }
    }
  }
}

// this is the only way to add snippets to SnippetRegistry, aka m_snippets
void
SnippetRegistry::add(Snippet *s) {
  std::cout << "[SnippetRegistry::add]" << std::endl;
  std::cout << "\tType: " << s->GetType() << std::endl;
  std::cout << "\tName: " << s->GetName() << std::endl;
  std::cout << "\tKeywords: ";
  m_snippets.insert(s);
  std::set<std::string> keywords = s->GetKeywords();
  for (auto it=keywords.begin();it!=keywords.end();it++) {
    std::cout << *it << ", ";
    if (m_id_map.find(*it) == m_id_map.end()) {
      m_id_map[*it] = std::set<Snippet*>();
    }
    m_id_map[*it].insert(s);
  }
  std::cout<<std::endl;
}

void
SnippetRegistry::addDependence(Snippet *from, Snippet *to) {
  if (m_dependence_map.find(from) == m_dependence_map.end()) {
    m_dependence_map[from] = std::set<Snippet*>();
  }
  m_dependence_map[from].insert(to);
}

void
SnippetRegistry::addDependence(Snippet *from, std::set<Snippet*> to) {
  for (auto it=to.begin();it!=to.end();it++) {
    addDependence(from, *it);
  }
}

static std::regex structure_reg("^typedef\\s+struct(\\s+\\w+)?\\s*{");
static std::regex enum_reg("^typedef\\s+enum(\\s+\\w+)?\\s*{");
static std::regex union_reg("^typedef\\s+union(\\s+\\w+)?\\s*{");
bool is_structure(const std::string& code) {
  if (std::regex_search(code, structure_reg)) return true;
  else return false;
}
bool is_enum(const std::string& code) {
  if (std::regex_search(code, enum_reg)) return true;
  else return false;
}
bool is_union(const std::string& code) {
  if (std::regex_search(code, union_reg)) return true;
  else return false;
}

Snippet*
SnippetRegistry::createSnippet(const CtagsEntry& ce) {
  std::cout << "[SnippetRegistry::createSnippet] " << ce.GetName() << " " << ce.GetType() << std::endl;
  Snippet *s;
  std::string code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
  std::string trimed_code = code;
  StringUtil::trim(trimed_code);
  std::string filename = ce.GetFileName();
  // only the last component of filename. Used for dependence resolving
  if (filename.find("/") != -1) {
    filename = filename.substr(filename.rfind("/")+1);
  }
  int line_number = ce.GetLineNumber();
  std::string id = ce.GetName();
  switch (ce.GetType()) {
    case 'f': s = new FunctionSnippet(trimed_code, id, filename, line_number); return s; break;
    case 's': s = new StructureSnippet(trimed_code, filename, line_number); return s; break;
    case 'e':
    case 'g': s = new EnumSnippet(trimed_code, filename, line_number); return s; break;
    case 'u': s = new UnionSnippet(trimed_code, filename, line_number); return s; break;
    case 'd': s = new DefineSnippet(trimed_code, filename, line_number); return s; break;
    case 'v': s = new VariableSnippet(trimed_code, id, filename, line_number); return s; break;
    // do not consider the following two cases
    // case 'c': constant
    // case 'm': member fields of a structure
    case 't': {
      Snippet *s;
      if (is_structure(trimed_code)) {
        s = new StructureSnippet(trimed_code, filename, line_number);
      } else if (is_enum(trimed_code)) {
        s = new EnumSnippet(trimed_code, filename, line_number);
      } else if (is_union(trimed_code)) {
        s = new UnionSnippet(trimed_code, filename, line_number);
      } else {
        s = new TypedefSnippet(trimed_code, id, filename, line_number);
      }
      return s;
      break;
    }
    default: return NULL;
  }
  return s;
}
