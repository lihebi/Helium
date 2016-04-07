#include <pugixml.hpp>

#include <iostream>
#include <boost/regex.hpp>

#include "snippet.h"
#include "utils.h"
#include "resolver.h"
#include "arg_parser.h"
#include "options.h"

using namespace utils;

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


#if 0
/**
 * This will create (new) something. Be sure to release them.
 */
std::set<Snippet*> create_snippets(std::vector<CtagsEntry> entries) {
  std::set<Snippet*> ret;
  if (!entries.empty()) {
    for (auto it=entries.begin();it!=entries.end();it++) {
      CtagsEntry ce = *it;
      Snippet *s = new Snippet(ce);
      if (s==NULL) continue;
      if (!s->IsValid()) {
        delete s;
        continue;
      }
      ret.insert(s);
    }
  }
  return ret;
}

/**
 * The new resolving system.
 * Do not record dependencies at the first time.
 * Purely do the resolving.
 * Use a worklist algorithm to record the size, so that we get a view of how many remains.
 */

std::set<Snippet*> SnippetRegistry::Resolve(const std::string& name, std::set<SnippetKind> kinds) {
  // hit
  if (!lookUp(name, kinds).empty()) {
    return lookUp(name, kinds);
  }
  std::set<Snippet*> ret;
  std::set<std::string> worklist;
  worklist.insert(name);
  std::set<std::string> avoid;
  while (!worklist.empty()) {
    // debug_time("1");
    // std::cout <<worklist.size()  << "\n";
    // for (std::string id : worklist) {
    //   std::cout <<id << " ";
    // }
    // std::cout  << "\n";
    std::string name = *worklist.begin();
    worklist.erase(name);
    if (avoid.count(name) == 1) continue;
    avoid.insert(name);
    // debug_time("2");
    if (!lookUp(name, kinds).empty()) continue;
    // debug_time("3");
    std::vector<CtagsEntry> entries = ctags_parse(name);
    // debug_time("6");
    std::set<Snippet*> snippets = create_snippets(entries);
    // debug_time("4");
    for (auto it=snippets.begin(), end=snippets.end(); it!=end; ++it) {
      m_snippets.insert(*it);
      // also take care of m_id_map and m_dependence_map
      std::set<std::string> keys = (*it)->GetSignatureKey();
      if (PrintOption::Instance()->Has(POK_AddSnippet)) {
        std::cout <<"Adding snippet: ";
        std::cout << (*it)->ToString()  << "\n";
      }
      if (PrintOption::Instance()->Has(POK_AddSnippetDot)) {
        std::cout <<"." << std::flush;
      }
      for (std::string key : keys) {
        m_id_map[key].insert(*it);
      }
    }
    // debug_time("5");
  
    for (auto it=snippets.begin(), end=snippets.end(); it!=end; ++it) {
      std::string code = (*it)->GetCode();
      std::set<std::string> ids = extract_id_to_resolve(code);
      // std::set<std::string> ids = get_to_resolve(code);
      worklist.insert(ids.begin(), ids.end());
    }
  }
  return ret;


  
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
}

#endif
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

#if true
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
  } else {
    // printing out unresolved ones
    // not useful at all. Do not use this.
    if (PrintOption::Instance()->Has(POK_UnresolvedID)) {
      utils::print(name, utils::CK_Yellow);
    }
  }
  if (direct_snippets.empty()) return result;
  /*******************************
   * remove duplicate in direct_snippets
   *******************************/
  // std::set<std::string> codes;
  // std::set<Snippet*> to_remove;
  // for (Snippet *s : direct_snippets) {
  //   // std::cout << "**********" << "\n";
  //   // std::cout << s->MainName() << "\n";
  //   // std::cout <<snippet_kind_to_char(s->MainKind())  << "\n";
  //   // for (std::string key : s->GetSignatureKey()) {
  //   //   std::cout << key;
  //   //   for (SnippetKind k : s->GetSignature(key)) {
  //   //     std::cout << snippet_kind_to_char(k);
  //   //   }
  //   // }
  //   // std::cout << "\n";
  //   // std::cout <<s->GetCode()  << "\n";
  //   if (codes.find(s->GetCode()) != codes.end()) {
  //     to_remove.insert(s);
  //   } else {
  //     codes.insert(s->GetCode());
  //   }
  // }
  // for (Snippet *s : to_remove) {
  //   direct_snippets.erase(s);
  //   delete s;
  // }

  // get the specific kinds. NOTE: this is the only place that check the kinds
  // this result is used for return
  // everything follows that will not be related to result, but to keep resolving
  // may need an approach to stop if takes too much time? NO!
  // actually they will become the dependencies.
  // TODO need some test cases to show it is robust enough and does not do something many times.
  for (auto it=direct_snippets.begin();it!=direct_snippets.end();++it) {
    if ((*it)->SatisfySignature(name, kinds)) {
      result.insert(*it);
    }
  }

  /**
   * Remove snippets with the same signature
   */
  std::set<SnippetSignature> sig_filter;
  std::set<Snippet*> to_remove;
  for (auto it=direct_snippets.begin(), end=direct_snippets.end();it!=end;it++) {
    Snippet *s = *it;
    SnippetSignature sig = s->GetSignature();
    if (sig_filter.count(sig) == 1) {
      // FIXME
      // it = direct_snippets.erase(it);
      // delete s;
      to_remove.insert(s);
    } else {
      sig_filter.insert(sig);
    }
  }
  // for (Snippet* s : to_remove) {
  //   if (direct_snippets.count(s) == 1) {
  //     direct_snippets.erase(s);
  //     delete s;
  //   }
  // }

  // std::cout << "------"  << "\n";
  // recursively resolve and add to local storage(m_snippets).
  // stored pointers and will never be freed.
  // This is where the snippet is added.
  for (auto it=direct_snippets.begin(), end=direct_snippets.end(); it!=end; ++it) {
    m_snippets.insert(*it);
    // also take care of m_id_map and m_dependence_map
    std::set<std::string> keys = (*it)->GetSignatureKey();
    if (PrintOption::Instance()->Has(POK_AddSnippet)) {
      std::cout <<"Adding snippet: ";
      std::cout << (*it)->ToString()  << "\n";
      // std::string ss = (*it)->ToString();
      // if (ss == "{}") {
      //   std::cout << "empty!"  << "\n";
      //   std::cout << (*it)->GetCode()  << "\n";
      // }
    }
    if (PrintOption::Instance()->Has(POK_AddSnippetDot)) {
      std::cout <<"." << std::flush;
    }
    for (std::string key : keys) {
      m_id_map[key].insert(*it);
    }
  }
  
  // recursive
  for (auto it=direct_snippets.begin(), end=direct_snippets.end(); it!=end; ++it) {
    std::string code = (*it)->GetCode();
    std::set<std::string> ids = extract_id_to_resolve(code);
    // std::set<std::string> ids = get_to_resolve(code);
    for (std::string id : ids) {
      std::set<Snippet*> snippets = Resolve(id);
      addDeps(*it, snippets);
    }
  }
  return result;
}
#endif


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
    SnippetSignature sig = s->GetSignature();
    if (sig.empty()) {
      result += "empty**";
      result += s->GetCode();
    }
    for (auto m : sig) {
      std::string key = m.first;
      // SnippetKind k = m.second;
      result += "key: " + key + " kinds: ";
      for (SnippetKind k : sig[key]) {
        result += snippet_kind_to_char(k);
      }
      result += '\n';
    }
    result += "-------\n";
  }
  return result;
}
