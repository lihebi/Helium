#include "header_dep.h"
#include "utils/utils.h"
#include "snippet_db.h"
#include <iostream>

/**
 * Deprecated! Not used!
 */
HeaderDep *HeaderDep::m_instance = NULL;
static boost::regex include_reg("#\\s*include\\s*\"(\\w+\\.h)\"");

std::map<std::string, std::set<std::string> > get_deps(std::string folder) {
  std::map<std::string, std::set<std::string> > ret;
  std::vector<std::string> headers;
  utils::get_files_by_extension(folder, headers, "h");
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
          ret[filename].insert(new_file);
        }
      }
      is.close();
    }
  }
  return ret;
}

void dump_to_db(std::map<std::string, std::set<std::string> > &deps) {
  for (auto m : deps) {
    std::string from = m.first;
    for (const std::string &to : m.second) {
      SnippetDB::Instance()->InsertHeaderDep(from, to);
    }
  }
}

void HeaderDep::Create(std::string benchmark_folder) {
  std::map<std::string, std::set<std::string> > deps;
  deps = get_deps(benchmark_folder);
  dump_to_db(deps);
}


void HeaderDep::LoadFromDB() {
  std::vector<std::pair<std::string, std::string> > deps;
  deps = SnippetDB::Instance()->GetHeaderDep();
  for (auto m : deps) {
    std::string from = m.first;
    std::string to = m.second;
    m_dep_m[from].insert(to);
  }
}


bool HeaderDep::isDependOn(std::string lhs, std::string rhs) {
  if (lhs.find("/")!=std::string::npos) lhs = lhs.substr(lhs.rfind("/")+1);
  if (rhs.find("/")!=std::string::npos) rhs = rhs.substr(rhs.rfind("/")+1);
  if (m_dep_m.find(lhs) != m_dep_m.end()) {
    std::set<std::string> ss = m_dep_m[lhs];
    if (ss.find(rhs) != ss.end()) {
      return true;
    }
  }
  return false;
}

bool HeaderDep::sortOneRound(std::vector<std::string> &sorted) {
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
std::vector<std::string> HeaderDep::Sort(std::set<std::string> headers) {
  std::vector<std::string> sorted_headers;
  for (auto it=headers.begin();it!=headers.end();it++) {
    sorted_headers.push_back(*it);
  }
  while(sortOneRound(sorted_headers)) ;
  return sorted_headers;
}

void HeaderDep::Dump() {
  for (auto &v : m_dep_m) {
    std::cout << v.first  << " => \n";
    for (const std::string &s : v.second) {
      std::cout << "\t" << s << "\n";
    }
  }
}
