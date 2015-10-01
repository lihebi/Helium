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


static std::regex structure_reg("^typedef\\s+struct(\\s+\\w+)?\\s*\\{");
static std::regex enum_reg     ("^typedef\\s+enum(\\s+\\w+)?\\s*\\{");
static std::regex union_reg    ("^typedef\\s+union(\\s+\\w+)?\\s*\\{");

static char
get_true_type(const CtagsEntry& ce) {
  if (ce.GetType() == 't') {
    std::string code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
    std::string trimed_code = code;
    StringUtil::trim(trimed_code);
    if      (std::regex_search(trimed_code, structure_reg)) return 's';
    else if (std::regex_search(trimed_code, enum_reg)) return 'g';
    else if (std::regex_search(trimed_code, union_reg)) return 'u';
    else return 't';
  } else if (ce.GetType() == 'e') return 'g';
  else return ce.GetType();
}

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
  // lookup to save computation
  // FIXME may not be the first .. but since we have type insure, it should be no problem.
  Snippet *s = LookUp(ce.GetName(), get_true_type(ce));
  if (s) return s;
  s = createSnippet(ce);
  if (!s) return NULL;
  add(s);
  resolveDependence(s);
  return s;
}

void
SnippetRegistry::resolveDependence(Snippet *s) {
  std::set<std::string> ss = Resolver::ExtractToResolve(s->GetCode());
  // std::cout << "[SnippetRegistry::resolveDependence] size of to resolve: " << ss.size() << std::endl;
  for (auto it=ss.begin();it!=ss.end();it++) {
    if (!LookUp(*it).empty()) {
      // already resolved. Just add dependence
      addDependence(s, LookUp(*it));
    } else {
      // resolve by ctags
      // FIXME These are duplicate code. And possibly wrong because miss many checks
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



Snippet*
SnippetRegistry::createSnippet(const CtagsEntry& ce) {
  Snippet *s;
  char t = get_true_type(ce);
  switch (t) {
    case 'f': s = new FunctionSnippet(ce); return s; break;
    case 's': s = new StructureSnippet(ce); return s; break;
    case 'e':
    case 'g': s = new EnumSnippet(ce); return s; break;
    case 'u': s = new UnionSnippet(ce); return s; break;
    case 'd': s = new DefineSnippet(ce); return s; break;
    case 'v': s = new VariableSnippet(ce); return s; break;
    // do not consider the following two cases
    // case 'c': constant
    // case 'm': member fields of a structure
    case 't': s = new TypedefSnippet(ce); return s; break;
    default: return NULL;
  }
  return s;
}
