#ifndef HEADER_RESOLVER_H
#define HEADER_RESOLVER_H

#include "helium/utils/common.h"

class HeaderResolver {
public:
  static HeaderResolver* Instance() {
    if (m_instance == 0) {
      m_instance = new HeaderResolver();
    }
    return m_instance;
  }
  // load all the header files inside the folder recursively,
  // scan the #inlcude "" statement, and get dependence relations between them
  void Load(const std::string& folder);
  // sort the headers by dependence
  std::vector<std::string> Sort(std::set<std::string> headers);
  void Dump();
  void DumpDeps();
  // check if a header is included in the original benchmark
  bool IsIncluded(std::string header) {
    if (header.find("/") != std::string::npos) {
      header = header.substr(header.rfind("/")+1);
    }
    if (m_headers.count(header) == 1) return true;
    else return false;
  }

  /**
   * Used Headers in the project.
   */
  std::set<std::string> GetUsedHeaders() {
    return m_headers;
  }
private:
  bool sortOneRound(std::vector<std::string> &sorted);
  bool isDependOn(std::string lhs, std::string rhs);
  // void addDependence(const std::string& lhs, const std::string& rhs);
  // void implicit(std::string folder);
  HeaderResolver();
  ~HeaderResolver();
  static HeaderResolver* m_instance;

  // std::vector<std::string> m_headers;
  std::map<std::string, std::set<std::string> > m_hard_deps_map;
  std::map<std::string, std::set<std::string> > m_soft_deps_map;

  // all included headers in the project
  std::set<std::string> m_headers;
};



#endif /* HEADER_RESOLVER_H */
