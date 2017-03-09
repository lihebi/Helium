#ifndef SYSTEM_RESOLVER_H
#define SYSTEM_RESOLVER_H

#include "helium/utils/common.h"
#include "helium/resolver/snippet.h"

/**
 * FIXME This System Resolver Seems to be broken!
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
  // std::string GetHeaders() const;
  std::string GetLibs() const;
  static void check_headers();

  /**
   * Get available headers on the system.
   */
  std::set<std::string> GetAvailableHeaders() {
    return m_headers;
  }
private:
  void parseHeaderConf(std::string file);
  SystemResolver();
  ~SystemResolver() {}
  // std::vector<Header> m_headers; // header files used
  static SystemResolver* m_instance;
  tagFile *m_tagfile = NULL;
  tagEntry *m_entry = NULL;
  // headers that need to be included
  std::set<std::string> m_headers;
  std::set<std::string> m_libs; // library compilation flags
};


#endif /* SYSTEM_RESOLVER_H */
