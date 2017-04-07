#include "helium/parser/HeaderManager.h"

#include "helium/utils/string_utils.h"

#include "helium/resolver/SnippetV2.h"

#include <regex>

using namespace v2;

using std::string;
using std::vector;
using std::set;
using std::map;
using std::pair;

HeaderManager *HeaderManager::instance = nullptr;

static bool has_suffix(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static std::set<std::string> filter_suffix(std::set<std::string> &all, std::string sub) {
  std::set<std::string> ret;
  std::string str,suffix;
  for (std::string s : all) {
    if (has_suffix(s, sub)) {
      ret.insert(s);
    }
  }
  return ret;
}




bool HeaderManager::header_exists(const std::string header) {
  for (std::string s : Includes) {
    fs::path p(s + "/" + header);
    if (fs::exists(p)) return true;
  }
  return false;
}

void HeaderManager::addConf(fs::path file) {
  std::map<std::string, std::string> headers = parseHeaderConf(file);
  for (auto m : headers) {
    if (header_exists(m.first)) {
      Headers.insert(m.first);
      if (!m.second.empty()) {
        // Libs.insert(m.second);
        Header2LibMap[m.first] = m.second;
      }
    } else {
      NonExistHeaders.insert(m.first);
    }
  }
}


void HeaderManager::dump(std::ostream &os) {
  os << "[HeaderManager] Headers: ";
  for (std::string s : Headers) {
    os << s << " ";
  }
  os << "\n";
  // os << "[HeaderManager] Libs: ";
  // for (std::string s : Libs) {
  //   os << s << " ";
  // }
  // os << "\n";
  os << "[HeaderManager] Includes: ";
  for (std::string s : Includes) {
    os << s << " ";
  }
  os << "\n";
  os << "[HeaderManager] Non-exist headers: ";
  for (std::string s : NonExistHeaders) {
    os << s << " ";
  }
  os << "\n";
}

std::set<std::string> HeaderManager::getHeaders() {
  std::set<std::string> ret;
  for (std::string s : Headers) {
    if (Mask.count(s) == 1) {
      ret.insert(s);
    }
  }
  ret.insert(ForceHeaders.begin(), ForceHeaders.end());
  return ret;
}

std::set<std::string> HeaderManager::getLibs() {
  std::set<std::string> headers = getHeaders();
  std::set<std::string> ret;
  for (std::string h : headers) {
    if (Header2LibMap.count(h) == 1) {
      ret.insert(Header2LibMap[h]);
    }
  }
  return ret;
}


std::map<std::string, std::string> HeaderManager::parseHeaderConf(fs::path file) {
  std::ifstream is;
  is.open(file.string());
  assert(is.is_open());
  std::string line;
  std::string flag;
  // from header name to compile flag
  std::map<std::string, std::string> headers;
  while (getline(is, line)) {
    utils::trim(line);
    flag = "";
    if (line.empty()) continue;
    if (line[0] == '#') {
      std::vector<std::string> v = utils::split(line);
      std::string s = v[0];
      if (s == "#INC") {
        // Includes.insert(s);
        assert(v.size() == 2);
        Includes.insert(v[1]);
      }
    } else {
      if (line.find(' ') != std::string::npos) {
        flag = line.substr(line.find(' '));
        line = line.substr(0, line.find(' '));
        utils::trim(flag);
      }
      if (line[0] == '!') {
        // force include
        line = line.substr(1);
        ForceHeaders.insert(line);
      }
      headers[line] = flag;
    }
  }
  return headers;
}


static std::regex include_reg("#\\s*include\\s*[\"<]([\\w/]+\\.h)[\">]");
static std::regex include_quote_reg("#\\s*include\\s*\"([\\w/]+\\.h)\"");
static std::regex include_angle_reg("#\\s*include\\s*<([\\w/]+\\.h)>");


/**
 * dir :: The directory of the original benchmark
 * replace :: cpp folder
 */
void HeaderManager::parseBench(fs::path dir) {
  // 1. mask
  // 2. header dependencies
  Mask.clear();
  Deps.clear();
  std::set<std::string> all_files;
  {
    // get all files
    fs::recursive_directory_iterator it(dir), eod;
    BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
      if (p.extension() == ".h" || p.extension() == ".c") {
        all_files.insert(p.string());
      }
    }
  }
  fs::recursive_directory_iterator it(dir), eod;
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (p.extension() == ".h" || p.extension() == ".c") {
      std::ifstream ifs(p.string());
      assert(ifs.is_open());
      std::string line;
      while (std::getline(ifs, line)) {
        std::smatch match;
        // quotes "a.h"
        if (std::regex_search(line, match, include_quote_reg)) {
          std::string file = match[1];
          std::set<std::string> matches = filter_suffix(all_files, file);
          for (std::string to : matches) {
            // both first and seconds are ABSOLUTE path
            Deps[p.string()].insert(to);
          }
        }
        // angle <a.h>
        if (std::regex_search(line, match, include_angle_reg)) {
          std::string file = match[1];
          all_headers.insert(file);
          Mask.insert(file);
          if (Headers.count(file) == 0) {
            if (header_exists(file)) {
              all_missed_exist_headers.insert(file);
            } else {
              all_missed_non_exist_headers.insert(file);
            }
          }
        }
      }
    }
  }
}
void HeaderManager::adjustDeps(std::string pattern, std::string replace) {
  std::map<std::string, std::set<std::string> > tmp;
  for (auto &m : Deps) {
    std::string from = m.first;
    for (std::string to : m.second) {
      assert(from.find(pattern) == 0);
      assert(to.find(pattern) == 0);
      from = from.replace(0, pattern.length(), replace);
      to = to.replace(0, pattern.length(), replace);
      tmp[from].insert(to);
    }
  }
  Deps = tmp;
}
