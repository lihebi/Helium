#include <pugixml.hpp>

#include <iostream>
#include <boost/regex.hpp>

#include "snippet.h"
#include "utils.h"
#include "resolver.h"
#include "arg_parser.h"

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
  std::vector<CtagsEntry> entries = ctags_parse(name); // parse ctags
  // construct all snippets that is directly related to name(the entries returned by ctags lookup), for ALL kinds
  if (!entries.empty()) {
    for (auto it=entries.begin();it!=entries.end();it++) {
      CtagsEntry ce = *it;
      Snippet *s = new Snippet(ce);
      if (s==NULL) continue;
      if (!s->IsValid()) {
        delete s;
        continue;
      }
      direct_snippets.insert(s);
    }
  }
  /*******************************
   * remove duplicate in direct_snippets
   *******************************/
  std::set<std::string> codes;
  std::set<Snippet*> to_remove;
  for (Snippet *s : direct_snippets) {
    // std::cout << "**********" << "\n";
    // std::cout << s->MainName() << "\n";
    // std::cout <<snippet_kind_to_char(s->MainKind())  << "\n";
    // for (std::string key : s->GetSignatureKey()) {
    //   std::cout << key;
    //   for (SnippetKind k : s->GetSignature(key)) {
    //     std::cout << snippet_kind_to_char(k);
    //   }
    // }
    // std::cout << "\n";
    // std::cout <<s->GetCode()  << "\n";
    if (codes.find(s->GetCode()) != codes.end()) {
      to_remove.insert(s);
    } else {
      codes.insert(s->GetCode());
    }
  }
  for (Snippet *s : to_remove) {
    direct_snippets.erase(s);
    // CAUTION: need to be freed if duplicate
    delete s;
  }

  // get the specific kinds. NOTE: this is the only place that check the kinds
  // this result is used for return
  // everything follows that will not be related to result, but to keep resolving
  // may need an approach to stop if takes too much time? NO!
  // actually they will become the dependencies.
  // FIXME need some test cases to show it is robust enough and does not do something many times.
  for (auto it=direct_snippets.begin();it!=direct_snippets.end();++it) {
    if ((*it)->SatisfySignature(name, kinds)) {
      result.insert(*it);
    }
  }
  
  // recursively resolve and add to local storage(m_snippets).
  // CAUTION: store pointers and will never be freed.
  // This is where the snippet is added.
  for (auto it=direct_snippets.begin(), end=direct_snippets.end(); it!=end; ++it) {
    m_snippets.insert(*it);
    // also take care of m_id_map and m_dependence_map
    std::set<std::string> keys = (*it)->GetSignatureKey();
    if (PrintOption::Instance()->Has(POK_AddSnippet)) {
      std::cout <<"Adding snippet: ";
      std::cout << (*it)->ToString()  << "\n";
    }
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


/**
 * Human readable printing, for debug.
 */
std::string SnippetRegistry::ToString() const {
  std::string result;
  result += "total snippet: " + std::to_string(m_snippets.size()) +'\n';
  for (auto m : m_id_map) {
    std::string key = m.first;
    std::set<Snippet*> snippets = m.second;
    result += "key: " + key + " kinds: ";
    for (Snippet* s : snippets) {
      std::set<SnippetKind> kinds = s->GetSignature(key);
      for (SnippetKind k : kinds) {
        result += snippet_kind_to_char(k);
      }
    }
    result += '\n';
  }
  result += "=========\n";
  for (auto s : m_snippets) {
    snippet_signature sig = s->GetSignature();
    if (sig.empty()) {
      result += "empty**";
      result += s->GetCode();
    }
    for (auto m : sig) {
      std::string key = m.first;
      SnippetKind k = m.second;
      result += "key: " + key + " kinds: " + snippet_kind_to_char(k);
      result += '\n';
    }
    result += "-------\n";
  }
  return result;
}
