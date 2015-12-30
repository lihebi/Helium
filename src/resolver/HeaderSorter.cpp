#include "resolver/HeaderSorter.hpp"
#include "util/FileUtil.hpp"
#include <fstream>
#include <boost/regex.hpp>
#include <iostream>

HeaderSorter* HeaderSorter::m_instance = 0;

static boost::regex include_reg("#\\s*include\\s*\"(\\w+\\.h)\"");

// load all the header files inside the folder recursively,
// scan the #inlcude "" statement, and get dependence relations between them
void
HeaderSorter::Load(const std::string& folder) {
  std::vector<std::string> headers;
  FileUtil::GetFilesByExtension(folder, headers, "h");
  for (auto it=headers.begin();it!=headers.end();it++) {
    std::ifstream is;
    std::string filename = *it;
    // get only the last component(i.e. filename) in the file path
    filename = filename.substr(filename.rfind("/")+1);
    is.open(*it);
    if (is.is_open()) {
      std::string line;
      while(std::getline(is, line)) {
        // process line
        boost::smatch match;
        if (boost::regex_search(line, match, include_reg)) {
          std::string new_file = match[1];
          // the filename part of including
          if (new_file.find("/") != std::string::npos) {
            new_file = new_file.substr(new_file.rfind("/")+1);
          }
          // add the dependence
          // FIXME if the include is in the middle of the header file,
          // The dependence may change.
          addDependence(filename, new_file);
        }
      }
      is.close();
    }
  }
}

void
HeaderSorter::addDependence(const std::string& lhs, const std::string& rhs) {
  if (m_dependence_map.find(lhs) == m_dependence_map.end()) {
    m_dependence_map[lhs] = std::set<std::string>();
  }
  m_dependence_map[lhs].insert(rhs);
}

bool
HeaderSorter::isDependOn(const std::string& lhs, const std::string& rhs) {
  if (m_dependence_map.find(lhs) != m_dependence_map.end()) {
    std::set<std::string> ss = m_dependence_map[lhs];
    if (ss.find(rhs) != ss.end()) {
      return true;
    }
  }
  return false;
}

bool
HeaderSorter::sortOneRound(std::vector<std::string> &sorted) {
  bool changed = false;
  for (size_t i=0;i<sorted.size();i++) {
    for (size_t j=i+1;j<sorted.size();j++) {
      if (isDependOn(sorted[i], sorted[j])) {
        std::string tmp = sorted[i];
        sorted[i] = sorted[j];
        sorted[j] = tmp;
        changed = true;
      }
    }
  }
  return changed;
}

// sort the headers by dependence
std::vector<std::string>
HeaderSorter::Sort(std::set<std::string> headers) {
  std::vector<std::string> sorted_headers;
  for (auto it=headers.begin();it!=headers.end();it++) {
    sorted_headers.push_back(*it);
  }
  while(sortOneRound(sorted_headers)) ;
  return sorted_headers;
}
