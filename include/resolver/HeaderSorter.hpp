#ifndef __HEADER_SORTER_HPP__
#define __HEADER_SORTER_HPP__

#include <string>
#include <vector>
#include <set>
#include <map>

class HeaderSorter {
public:
  static HeaderSorter* Instance() {
    if (m_instance == 0) {
      m_instance = new HeaderSorter();
    }
    return m_instance;
  }
  // load all the header files inside the folder recursively,
  // scan the #inlcude "" statement, and get dependence relations between them
  void Load(const std::string& folder);
  // sort the headers by dependence
  std::vector<std::string> Sort(std::set<std::string> headers);
private:
  bool sortOneRound(std::vector<std::string> &sorted);
  bool isDependOn(const std::string& lhs, const std::string& rhs);
  void addDependence(const std::string& lhs, const std::string& rhs);
  HeaderSorter() {}
  ~HeaderSorter() {}
  static HeaderSorter* m_instance;

  // std::vector<std::string> m_headers;
  std::map<std::string, std::set<std::string> > m_dependence_map;
};

#endif
