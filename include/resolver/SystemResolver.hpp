#ifndef __SYSTEM_RESOLVER_HPP__
#define __SYSTEM_RESOLVER_HPP__

#include <vector>
#include <string>
#include <readtags.h>

#include "type/Type.hpp"
#include "snippet/Snippet.hpp"
#include "resolver/Ctags.hpp"

/*
 * Check if an identifier is a system function or type.
 */

class SystemResolver {
public:
  static SystemResolver* Instance() {
    if (m_instance == 0) {
      m_instance = new SystemResolver();
    }
    return m_instance;
  }
  // load the systype.tags file
  void Load(const std::string& filename);
  // resolve to primitive type
  std::string ResolveType(const std::string& type);
  std::vector<CtagsEntry> Parse(const std::string& name) ;
  std::vector<CtagsEntry> Parse(const std::string& name, const std::string& type);
  bool Has(const std::string& name);
private:
  SystemResolver() {}
  ~SystemResolver() {}
  // std::vector<Header> m_headers; // header files used
  static SystemResolver* m_instance;
  tagFile *m_tagfile;
  tagEntry *m_entry;
};

#endif
