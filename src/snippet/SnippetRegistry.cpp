#include "snippet/SnippetRegistry.hpp"

#include "snippet/FunctionSnippet.hpp"
#include "snippet/EnumSnippet.hpp"
#include "snippet/DefineSnippet.hpp"
#include "snippet/StructureSnippet.hpp"
#include "snippet/TypedefSnippet.hpp"
#include "snippet/UnionSnippet.hpp"
#include "snippet/VariableSnippet.hpp"

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
  std::set<Snippet*> all;
  std::set<Snippet*> to;
  to.insert(snippet);
  while (!to.empty()) {
    Snippet *s = *(to.begin());
    to.erase(s);
    all.insert(s);
    if (m_dependence_map.find(s) != m_dependence_map.end()) {
      std::set<Snippet*> vs = m_dependence_map[s];
      for (auto it=vs.begin();it!=vs.end();it++) {
        if (all.find(*it) == all.end() && to.find(*it) == to.end()) {
          to.insert(*it);
        }
      }
    }
  }
  return all;
}

/**************************
 ********** Add ***********
 **************************/

// add(or lookup) snippet, and return the pointer
Snippet*
SnippetRegistry::Add(const std::string& code) {
  // do not use this method.
  // always add some code with correct type
  return NULL;
}

Snippet*
SnippetRegistry::Add(const std::string& code, char type) {
  Snippet *s = CreateSnippet(code, type);
  // TODO look up for duplicate
  m_snippets.insert(s);
  return s;
}

void
SnippetRegistry::AddDependence(Snippet *from, Snippet *to) {}

void
SnippetRegistry::AddDependence(Snippet *from, std::set<Snippet*> to) {}

Snippet*
SnippetRegistry::CreateSnippet(const std::string& code, char type) {
  Snippet *s;
  switch (type) {
    case 'f': s = new FunctionSnippet(code); return s; break;
    case 's': s = new StructureSnippet(code); return s; break;
    case 'e':
    case 'g': s = new EnumSnippet(code); return s; break;
    case 'u': s = new UnionSnippet(code); return s; break;
    case 'd': s = new DefineSnippet(code); return s; break;
    case 'v': s = new VariableSnippet(code); return s; break;
    // case 'c':
    // case 'm':
    case 't': {
      // TODO typedef may be struct or other types, functions, etc.
      Snippet *s = new TypedefSnippet(code); return s; break;
    }
    default: return NULL;
  }
  return s;
}
