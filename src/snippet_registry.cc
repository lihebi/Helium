#include <pugixml.hpp>

#include <iostream>
#include <boost/regex.hpp>

#include "snippet.h"
#include "utils.h"

SnippetRegistry* SnippetRegistry::m_instance = 0;


/*******************************
 ** Resolve
 *******************************/

/**
 * @param[in] name id to resolve
 * @param[in] type a string, each
 */
std::set<Snippet*> Resolve(const std::string& name, std::set<enum ctags_type> types) {
  std::set<Snippet*> snippets;
  std::vector<CtagsEntry> entries = ctags_parse(name);
  if (!entries.empty()) {
    for (auto it=entries.begin();it!=entries.end();it++) {
      CtagsEntry ce = *it;
      Snippet *s = new Snippet(ce);
      if (s->SatisfyType(name, types)) {
        snippets.insert(s);
      }
    }
  }
  return snippets;
}

/**************************
 ******* Look Up **********
 **************************/
std::set<Snippet*>
SnippetRegistry::lookUp(const std::string& name) {
  if (m_id_map.find(name) != m_id_map.end()) {
    return m_id_map[name];
  }
  return std::set<Snippet*>();
}
// look up by type
Snippet*
SnippetRegistry::lookUp(const std::string& name, char type) {
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
SnippetRegistry::lookUp(const std::string& name, const std::string& type) {
  if (m_id_map.find(name) != m_id_map.end()) {
    std::set<Snippet*> vs = m_id_map[name];
    for (auto it=vs.begin();it!=vs.end();it++) {
      if (type.find((*it)->GetType()) == std::string::npos) {
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
  resolveDependence(s, 0);
  return s;
}

void
SnippetRegistry::resolveDependence(Snippet *s, int level) {
  // We should not limit here techniquelly, because we once we have the dependence break,
  // We have no way to resolve the dependence after the break
  // e.g. a => b => c => d => e
  // If we break on c, then we will not have d and e.
  // everytime we resolve a,b,c we know that it is already resolved, we will not try to resolve again.
  std::set<std::string> ss = Resolver::ExtractToResolve(s->GetCode());
  for (auto it=ss.begin();it!=ss.end();it++) {
    if (!LookUp(*it).empty()) {
      // this will success if any snippet for a string is added.
      // So add all snippets for one string, then call this function recursively.
      addDependence(s, LookUp(*it));
    } else {
      std::vector<CtagsEntry> vc = Ctags::Instance()->Parse(*it);
      if (!vc.empty()) {
        std::set<Snippet*> added_snippets;
        // first: create and add, remove duplicate if possible
        for (auto jt=vc.begin();jt!=vc.end();jt++) {
          Snippet *s = LookUp(jt->GetName(), get_true_type(*jt));
          if (s) continue;
          Snippet *snew = createSnippet(*jt);
          if (snew) {
            add(snew);
            added_snippets.insert(snew);
          }
        }
        // then: add dependence adn resolve dependence
        addDependence(s, added_snippets);
        for (auto jt=added_snippets.begin();jt!=added_snippets.end();jt++) {
          resolveDependence(*jt, level+1);
        }
      }
    }
  }
}

// this is the only way to add snippets to SnippetRegistry, aka m_snippets
void
SnippetRegistry::add(Snippet *s) {
  m_snippets.insert(s);
  std::set<std::string> keywords = s->GetKeywords();
  for (auto it=keywords.begin();it!=keywords.end();it++) {
    if (m_id_map.find(*it) == m_id_map.end()) {
      m_id_map[*it] = std::set<Snippet*>();
    }
    m_id_map[*it].insert(s);
  }
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

