#ifndef __SNIPPET_REGISTRY_HPP__
#define __SNIPPET_REGISTRY_HPP__

#include "snippet/Snippet.hpp"

class SnippetRegistry {
public:
  static SnippetRegistry* Instance();
  const Snippet& LookUp() const;
private:
  SnippetRegistry();
  static SnippetRegistry* m_instance;
  // this is where actually store the resolved code snippets
  std::vector<Snippet> m_snippets;
};

#endif
