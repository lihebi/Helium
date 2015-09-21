#ifndef __SNIPPET_REGISTRY_HPP__
#define __SNIPPET_REGISTRY_HPP__

#include "snippet/Snippet.hpp"
#include <map>
#include <set>

class SnippetRegistry {
public:
  static SnippetRegistry* Instance() {
    if (m_instance == 0) {
      m_instance = new SnippetRegistry();
    }
    return m_instance;
  }
  std::set<Snippet*> LookUp(const std::string& name);
  // look up by type
  Snippet* LookUp(const std::string& name, char type);
  // look up by types
  std::set<Snippet*> LookUp(const std::string& name, const std::string& type);

  std::set<Snippet*> GetDependence(Snippet* snippet);
  // recursively get dependence
  std::set<Snippet*> GetAllDependence(Snippet* snippet);
  std::set<Snippet*> GetAllDependence(std::set<Snippet*> snippets);
  // add(or lookup) snippet, and return the pointer
  // this is public API.
  // add any code snippet will resolve the dependence
  // add will look up first to ensure there's no duplicate
  Snippet* Add(const std::string& code, char type, const std::string& id);



private:
  Snippet* createSnippet(const std::string& code, char type, const std::string& id);
  // Can not add dependence outside the class
  void addDependence(Snippet *from, Snippet *to);
  void addDependence(Snippet *from, std::set<Snippet*> to);
  void resolveDependence(Snippet *s);
  void add(Snippet *s);
  SnippetRegistry() {}
  // resolve dependence
  static SnippetRegistry* m_instance;
  // this is where actually store the resolved code snippets
  std::set<Snippet*> m_snippets;
  /*
   * id is the simplest form, without any keywords
   * keywords should be in the type field of the snippet
   */
  std::map<std::string, std::set<Snippet*> > m_id_map;
  /*
   * since every snippet is created and allocated in SnippetRegistry,
   * the address of the pointers should be unique
   * so we map from pointer to pointer vector, representing the dependence
   */
  std::map<Snippet*, std::set<Snippet*> > m_dependence_map;
};

#endif
