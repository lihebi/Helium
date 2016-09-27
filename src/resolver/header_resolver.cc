#include "header_resolver.h"
#include "utils/utils.h"
#include "utils/log.h"
#include <cassert>
#include <iostream>

using namespace utils;

HeaderResolver* HeaderResolver::m_instance = 0;

static boost::regex include_reg("#\\s*include\\s*[\"<]([\\w/]+\\.h)[\">]");
static boost::regex include_quote_reg("#\\s*include\\s*\"([\\w/]+\\.h)\"");
static boost::regex include_angle_reg("#\\s*include\\s*<([\\w/]+\\.h)>");

// load all the header files inside the folder recursively,
// scan the #inlcude "" statement, and get dependence relations between them
void
HeaderResolver::Load(const std::string& folder) {
  helium_print_trace("HeaderResolver::Load");
  std::vector<std::string> headers;
  utils::get_files_by_extension(folder, headers, std::vector<std::string>({"h", "c"}));
  for (auto it=headers.begin();it!=headers.end();it++) {
    std::string filename = *it;
    // get only the last component(i.e. filename) in the file path
    filename = filename.substr(filename.rfind("/")+1);
    // std::cout << "====== opening:: " << filename  << "\n";
    std::ifstream is(*it);
    if (is.is_open()) {
      std::string line;
      while(std::getline(is, line)) {
        // if (!line.empty() && line.find("#include") != std::string::npos) {
        //   std::cout << line  << "\n";
        // }
        // process line
        boost::smatch match;
        if (boost::regex_search(line, match, include_reg)) {
          std::string new_file = match[1];
          // std::cout << new_file  << "\n";
          // the filename part of including
          m_headers.insert(new_file);
          if (new_file.find("/") != std::string::npos) {
            new_file = new_file.substr(new_file.rfind("/")+1);
          }
          // add the dependence
          // FIXME if the include is in the middle of the header file,
          // The dependence may change.
          // addDependence(filename, new_file);
          m_hard_deps_map[filename].insert(new_file);

          // Include "used" only in <>
          // boost::regex_search(line, match, include_angle_reg);
          // new_file = match[1];
          // if (new_file.find("/") != std::string::npos) {
          //   new_file = new_file.substr(new_file.rfind("/")+1);
          // }
        }
      }
      is.close();
    }
  }
}

HeaderResolver::HeaderResolver() {}
HeaderResolver::~HeaderResolver() {}


// /**
//  * The header relationship sometimes does not explictly said in header includes.
//  * e.g. matz-stream on github, "node.h" doesn't include anything, but it does use the type declared in "strm.h"
//  * In the code, all the files include node.h always include strm.h before node.h,
//  * This is the reason why the compiler doesn't complain
//  */
// void HeaderResolver::implicit(std::string folder) {
//   std::vector<std::string> files;
//   std::map<std::string, std::string> soft_deps;
//   get_files_by_extension(folder, files, "ch");
//   for (std::string file: files) {
//     std::vector<std::string> includes;
//     std::ifstream is;
//     is.open(file);
//     assert(is.is_open());
//     std::string line;
//     while (std::getline(is, line)) {
//       boost::smatch match;
//       if (boost::regex_search(line, match, include_reg)) {
//         std::string new_file = match[1];
//         // the filename part of including
//         if (new_file.find("/") != std::string::npos) {
//           new_file = new_file.substr(new_file.rfind("/")+1);
//         }
//         includes.push_back(new_file);
//       }
//     }
//     // inforce dependence
//     for (size_t i=0;i<includes.size();i++) {
//       for (size_t j=i+1;j<includes.size();j++) {
//         // addSoftDeps(includes[j], includes[i]); // j depends on i, because i comes first
//         m_soft_deps_map[includes[j]].insert(includes[i]);
//       }
//     }
//   }
// }

/**
 * Do some trasformation, i.e. keep only file name part.
 */
bool
HeaderResolver::isDependOn(std::string lhs, std::string rhs) {
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
HeaderResolver::sortOneRound(std::vector<std::string> &sorted) {
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
HeaderResolver::Sort(std::set<std::string> headers) {
  std::vector<std::string> sorted_headers;
  for (auto it=headers.begin();it!=headers.end();it++) {
    sorted_headers.push_back(*it);
  }
  while(sortOneRound(sorted_headers)) ;
  return sorted_headers;
}

void HeaderResolver::Dump() {
  std::cout << "-- Header deps:"  << "\n";
  for (auto &v : m_hard_deps_map) {
    std::cout << v.first  << " => \n";
    for (const std::string &s : v.second) {
      std::cout << "\t" << s << "\n";
    }
  }
  std::cout << "-- Headers used in this benchmark:"  << "\n";
  for (std::string s : m_headers) {
    std::cout << s  << "\n";
  }
}

void HeaderResolver::DumpDeps() {
  for (auto &v : m_hard_deps_map) {
    std::cout  << "\t";
    std::cout << v.first  << " => ";
    for (const std::string &s : v.second) {
      std::cout << " " << s << "";
    }
    std::cout  << "\n";
  }
}
