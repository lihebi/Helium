#ifndef __SYSTEM_RESOLVER_HPP__
#define __SYSTEM_RESOLVER_HPP__

#include <vector>
#include <string>
#include <readtags.h>

#include "type/Type.hpp"
#include "snippet/Snippet.hpp"
/*
 * Check if an identifier is a system function or type.
 */

class Header {
public:
  std::string GetPath() {return m_path;}
  std::string GetFlag() {return m_flag;}
private:
  std::string m_path;
  std::string m_flag;
};

class SystemResolver {
public:
  static SystemResolver* Instance() {
    if (m_instance == 0) {
      m_instance = new SystemResolver();
    }
    return m_instance;
  }
  void Load(const std::string& filename);
  // check whether id can be resolved
  // modify m_headers and m_flags
  bool Check(const std::string& id);
  // resolve to primitive type
  static Type ResolveType(const std::string& type);
private:
  SystemResolver() {}
  ~SystemResolver() {}
  std::vector<Header> m_headers; // header files used
  static SystemResolver* m_instance;
  tagFile *m_tagfile;
  tagEntry *m_entry;
};

#endif
