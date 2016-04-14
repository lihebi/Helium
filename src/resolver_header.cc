#include "resolver.h"
#include "utils.h"
#include <cassert>
#include <iostream>

using namespace utils;

HeaderSorter* HeaderSorter::m_instance = 0;

static boost::regex include_reg("#\\s*include\\s*\"(\\w+\\.h)\"");

// load all the header files inside the folder recursively,
// scan the #inlcude "" statement, and get dependence relations between them
void
HeaderSorter::Load(const std::string& folder) {
  std::vector<std::string> headers;
  get_files_by_extension(folder, headers, "h");
  for (auto it=headers.begin();it!=headers.end();it++) {
    std::string filename = *it;
    // get only the last component(i.e. filename) in the file path
    filename = filename.substr(filename.rfind("/")+1);
    std::ifstream is(*it);
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
          // addDependence(filename, new_file);
          m_hard_deps_map[filename].insert(new_file);
        }
      }
      is.close();
    }
  }
  // TODO soft deps
  // implicit(folder);
  // completeDeps();
}

/**
 * The header relationship sometimes does not explictly said in header includes.
 * e.g. matz-stream on github, "node.h" doesn't include anything, but it does use the type declared in "strm.h"
 * In the code, all the files include node.h always include strm.h before node.h,
 * This is the reason why the compiler doesn't complain
 */
void HeaderSorter::implicit(std::string folder) {
  std::vector<std::string> files;
  std::map<std::string, std::string> soft_deps;
  get_files_by_extension(folder, files, "ch");
  for (std::string file: files) {
    std::vector<std::string> includes;
    std::ifstream is;
    is.open(file);
    assert(is.is_open());
    std::string line;
    while (std::getline(is, line)) {
      boost::smatch match;
      if (boost::regex_search(line, match, include_reg)) {
        std::string new_file = match[1];
        // the filename part of including
        if (new_file.find("/") != std::string::npos) {
          new_file = new_file.substr(new_file.rfind("/")+1);
        }
        includes.push_back(new_file);
      }
    }
    // inforce dependence
    for (size_t i=0;i<includes.size();i++) {
      for (size_t j=i+1;j<includes.size();j++) {
        // addSoftDeps(includes[j], includes[i]); // j depends on i, because i comes first
        m_soft_deps_map[includes[j]].insert(includes[i]);
      }
    }
  }
}

/**
 * lhs depends on rhs, i.e. rhs must preceed lhs
 * This is soft dependence, i.e. recommend, but strict.
 * Need to perform a check if it violates other deps.
 * If it violates a hard dep, discard it.
 * If it violates a soft dep, discard both (put them in the redzone).
 * If it violates the redzone, put it into the redzone
 * Redzone is some violated soft rules.
 * They should be kept for further violation detection.
 */

/**
 * Complete both hard and soft deps.
 * 1. for all the hard deps, add transitive closure
 * 2. for all soft ones, move all cross-violated and hard-violated ones into redzone.
 *   - cross-violate: two soft dep(even if they are put into red zone) violate each other
 *   - hard violate: violate a hard dep
 */
void HeaderSorter::completeDeps() {
  // transitive closure of hard deps
  /**
   * worklist. for each in worklist, traverse until 
   */
  // std::set<std::string> done;
  // std::set<std::string> worklist;
  // for (auto m : m_hard_deps_map) {
  //   worklist.insert(m.first);
  // }
  // while (!worklist.empty()) {
  //   std::string item = *worklist.begin();
  //   worklist.erase(worklist.begin());
  //   done.insert(item);
  //   std::set<std::string> tmp;
  //   for (std::string m : m_hard_deps_map[item]) {
  //     tmp.insert(m_hard_deps_map[m].begin(), m_hard_deps_map[m].end());
  //   }
  //   while (!tmp.empty()) {
  //     std::string s = *tmp.begin();
  //     tmp.erase(s);
  //     // if discovered a new 
  //     if (m_hard_deps_map[item].count(s) == 0) {
  //       m_hard_deps_map[item].insert(s);
  //       tmp.insert(m_hard_deps_map[s].begin(), m_hard_deps_map[s].end());
  //     }
  //   }
  // }
}

// void HeaderSorter::addSoftDeps(std::string lhs, std::string rhs) {
//   if (m_soft_deps_map.count(lhs) == 0) {
//     m_soft_deps_map[lhs] = std::set<std::string>();
//   }
//   m_soft_deps_map[lhs].insert(rhs);
// }

// void
// HeaderSorter::addDependence(const std::string& lhs, const std::string& rhs) {
//   if (m_dependence_map.find(lhs) == m_dependence_map.end()) {
//     m_dependence_map[lhs] = std::set<std::string>();
//   }
//   m_dependence_map[lhs].insert(rhs);
// }

/**
 * Do some trasformation, i.e. keep only file name part.
 */
bool
HeaderSorter::isDependOn(std::string lhs, std::string rhs) {
  if (lhs.find("/")!=std::string::npos) lhs = lhs.substr(lhs.rfind("/")+1);
  if (rhs.find("/")!=std::string::npos) rhs = rhs.substr(rhs.rfind("/")+1);
  if (m_hard_deps_map.find(lhs) != m_hard_deps_map.end()) {
    std::set<std::string> ss = m_hard_deps_map[lhs];
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

void HeaderSorter::Dump() {
  for (auto &v : m_hard_deps_map) {
    std::cout << v.first  << " => \n";
    for (const std::string &s : v.second) {
      std::cout << "\t" << s << "\n";
    }
  }
}
