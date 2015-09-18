#include "resolver/Resolver.hpp"
#include "snippet/SnippetRegistry.hpp"
#include "resolver/Ctags.hpp"
#include "util/FileUtil.hpp"

#include <iostream>

bool
Resolver::resolveLocal(std::string name) {
  std::vector<CtagsEntry> vc = Ctags::Instance()->Parse(name);
  if (vc.empty()) return false;
  else {
    for (auto it=vc.begin();it!=vc.end();it++) {
      std::string code = FileUtil::GetBlock(it->GetFileName(), it->GetLineNumber(), it->GetType());
      Snippet *s = SnippetRegistry::Instance()->Add(code, it->GetType());
      m_snippets.insert(s);
      Resolver new_resolver = Resolver(code);
      new_resolver.Resolve();
      std::set<Snippet*> snippets = new_resolver.GetSnippets();
      SnippetRegistry::Instance()->AddDependence(s, snippets);
    }
    return true;
  }
}
bool
Resolver::resolveSystem(std::string name) {
  // TODO
  return false;
}
void
Resolver::Resolve() {
  std::cout<<"[Resolver][Resolve]"<<std::endl;
  extractToResolve(m_code);
  for (auto it=m_to_resolve.begin();it!=m_to_resolve.end();it++) {
    std::set<Snippet*> known_snippets = SnippetRegistry::Instance()->LookUp(*it);
    if (!known_snippets.empty()) {
      for (auto it=known_snippets.begin();it!=known_snippets.end();it++) {
        m_snippets.insert(*it);
      }
      m_resolved.insert(*it);
    } else {
      if (resolveLocal(*it)) {
        m_resolved.insert(*it);
      } else if (resolveSystem(*it)) {
        m_resolved.insert(*it);
      } else {
        m_unresolved.insert(*it);
      }
    }
  }
}

void
Resolver::extractToResolve(const std::string& code) {
  std::smatch match;
  std::regex_search(code, match, m_regex);
  for (int i=0;i<match.size();i++) {
    if (m_unresolved.find(match[i]) == m_unresolved.end()
    && m_resolved.find(match[i]) == m_resolved.end()
    && c_common_keyword.find(match[i]) == c_common_keyword.end()) {
      m_to_resolve.insert(match[i]);
    }
  }
}
