#include <pugixml.hpp>

#include <iostream>
#include <boost/regex.hpp>

#include "snippet.h"
#include "utils.h"
#include "resolver.h"

SnippetRegistry* SnippetRegistry::m_instance = 0;


/*******************************
 ** Resolve
 *******************************/


// TODO need manually update if the enum changes. Any way to auto it?
static const std::set<SnippetKind> all_snippet_kinds = {
  SK_Function,
  SK_Structure,
  SK_Enum,
  SK_Union,
  SK_Define,
  SK_Variable,
  SK_EnumMember,
  SK_Typedef,
  SK_Const,
  SK_Member
};


std::set<Snippet*> SnippetRegistry::Resolve(const std::string& name) {
  return Resolve(name, all_snippet_kinds);
}

std::set<Snippet*> SnippetRegistry::Resolve(const std::string& name, SnippetKind kind) {
  std::set<SnippetKind> kinds;
  kinds.insert(kind);
  return Resolve(name, kinds);
}

/**
 * Main work-horse of Resolving.
 * Need to check if it is already in.
 * If yes, get the specific kind by lookup.
 * If not, resolve all IDs; Add them to index; return specific kinds.
 *
 * Resolve the id "name", return snippet if found, or empty set if cannot resolve.
 Internally, it resolve the id, for all types, recursively.
 Every time the lookup hit the record, that means for this entry, nothing needs to be done.
 We resolve everything at once.
 * This is the only API to add something into registry from outside.
 * Ctags resolver will not be directly used by client.
 */

std::set<Snippet*> SnippetRegistry::Resolve(const std::string& name, std::set<SnippetKind> kinds) {
  // hit
  if (!lookUp(name, kinds).empty()) {
    return lookUp(name, kinds);
  }
  // doesn't hit, resolve it!
  std::set<Snippet*> result;
  std::set<Snippet*> all_snippets;
  std::set<Snippet*> direct_snippets;
  std::vector<CtagsEntry> entries = ctags_parse(name);
  // construct all snippets that is directly related to name(the entries returned by ctags lookup), for ALL kinds
  if (!entries.empty()) {
    for (auto it=entries.begin();it!=entries.end();it++) {
      CtagsEntry ce = *it;
      Snippet *s = new Snippet(ce);
      direct_snippets.insert(s);
    }
  }
  // TODO remove duplicate in direct_snippets
  // CAUTION: need to be freed if duplicate
  ;
  // get the specific kinds. NOTE: this is the only place that check the kinds
  for (auto it=direct_snippets.begin();it!=direct_snippets.end();++it) {
    if ((*it)->SatisfySignature(name, kinds)) {
      result.insert(*it);
    }
  }
  // recursively resolve and add to local storage(m_snippets).
  // CAUTION: store pointers and will never be freed.
  for (auto it=direct_snippets.begin(), end=direct_snippets.end(); it!=end; ++it) {
    m_snippets.insert(*it);
    // also take care of m_id_map and m_dependence_map
    std::set<std::string> keys = (*it)->GetSignatureKey();
    for (std::string key : keys) {
      m_id_map[key].insert(*it);
    }
  }
  
  // recursive
  for (auto it=direct_snippets.begin(), end=direct_snippets.end(); it!=end; ++it) {
    std::string code = (*it)->GetCode();
    std::set<std::string> ids = extract_id_to_resolve(code);
    for (std::string id : ids) {
      std::set<Snippet*> snippets = Resolve(id);
      addDeps(*it, snippets);
    }
  }
  return result;
}


/**************************
 ** Look Up
 **************************/

std::set<Snippet*> SnippetRegistry::lookUp(const std::string& name) {
  // return lookUp(name, all_snippet_kinds);
  if (m_id_map.find(name) != m_id_map.end()) {
    return m_id_map[name];
  }
  return std::set<Snippet*>();
}
std::set<Snippet*> SnippetRegistry::lookUp(const std::string& name, SnippetKind kind) {
  std::set<SnippetKind> kinds = {kind};
  return lookUp(name, kinds);
}
std::set<Snippet*> SnippetRegistry::lookUp(const std::string& name, std::set<SnippetKind> kinds) {
  std::set<Snippet*> result;
  if (m_id_map.find(name) != m_id_map.end()) {
    // return m_id_map[name];
    std::set<Snippet*> snippets = m_id_map[name];
    for (auto it=snippets.begin();it!=snippets.end();++it) {
      if ((*it)->SatisfySignature(name, kinds)) {
        result.insert(*it);
      }
    }
  }
  return result;
}

/**************************
 ******* Dependence **********
 **************************/

std::set<Snippet*>
SnippetRegistry::GetDeps(Snippet* snippet) {
  if (m_dependence_map.find(snippet) != m_dependence_map.end()) {
    return m_dependence_map[snippet];
  } else {
    return std::set<Snippet*>();
  }
}
// recursively get dependence
std::set<Snippet*>
SnippetRegistry::GetAllDeps(Snippet* snippet) {
  std::set<Snippet*> snippets;
  snippets.insert(snippet);
  return GetAllDeps(snippets);
}

/*
 * Get all snippets dependence
 * return including the params
 */
std::set<Snippet*>
SnippetRegistry::GetAllDeps(std::set<Snippet*> snippets) {
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
    std::set<Snippet*> dep = GetDeps(tmp);
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

void
SnippetRegistry::addDep(Snippet *from, Snippet *to) {
  if (m_dependence_map.find(from) == m_dependence_map.end()) {
    m_dependence_map[from] = std::set<Snippet*>();
  }
  m_dependence_map[from].insert(to);
}

void
SnippetRegistry::addDeps(Snippet *from, std::set<Snippet*> to) {
  for (auto it=to.begin();it!=to.end();it++) {
    addDep(from, *it);
  }
}

