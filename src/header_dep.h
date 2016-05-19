#ifndef HEADER_DEP_H
#define HEADER_DEP_H

#include "common.h"

class HeaderDep {
public:
  ~HeaderDep() {}
  static HeaderDep *Instance() {
    if (!m_instance) {
      m_instance = new HeaderDep();
    }
    return m_instance;
  }
  static void Create(std::string benchmark_folder);
  void LoadFromDB();
  std::vector<std::string> Sort(std::set<std::string> headers);
  void Dump();
private:
  bool isDependOn(std::string lhs, std::string rhs);
  bool sortOneRound(std::vector<std::string> &sorted);
  HeaderDep() {}
  static HeaderDep *m_instance;
  std::map<std::string, std::set<std::string> > m_dep_m; // a.h includes b.h and c.h
};

#endif /* HEADER_DEP_H */
