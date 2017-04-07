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
      headers[line] = flag;
    }
  }
  return headers;
}


static std::regex include_reg("#\\s*include\\s*[\"<]([\\w/]+\\.h)[\">]");
static std::regex include_quote_reg("#\\s*include\\s*\"([\\w/]+\\.h)\"");
static std::regex include_angle_reg("#\\s*include\\s*<([\\w/]+\\.h)>");

void HeaderManager::mask(fs::path dir) {
  Mask.clear();
  std::set<std::string> ret;
  fs::recursive_directory_iterator it(dir), eod;
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (p.extension() == ".h" || p.extension() == ".c") {
      std::ifstream ifs(p.string());
      assert(ifs.is_open());
      std::string line;
      while (std::getline(ifs, line)) {
        std::smatch match;
        if (std::regex_search(line, match, include_angle_reg)) {
          std::string file = match[1];
          Mask.insert(file);
        }
      }
    }
  }

  // std::cout << "Mask size: " << Mask.size() << "\n";
  masked = true;
}
void HeaderManager::unmask() {
  masked = false;
}


std::set<std::string> HeaderManager::discoverHeader(fs::path dir) {
  /**
   * 1. loop through all files with .h and .c
   * 2. get #include <>
   * 3. search in system for existence
   */
  std::set<std::string> ret;
  fs::recursive_directory_iterator it(dir), eod;
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (p.extension() == ".h" || p.extension() == ".c") {
      std::ifstream ifs(p.string());
      assert(ifs.is_open());
      std::string line;
      while (std::getline(ifs, line)) {
        std::smatch match;
        if (std::regex_search(line, match, include_angle_reg)) {
          std::string file = match[1];
          if (Headers.count(file) == 0 && header_exists(file)) {
            ret.insert(file);
          }
        }
      }
      ifs.close();
    }
  }
  return ret;
}

std::set<std::string> HeaderManager::checkHeader(fs::path dir) {
  std::set<std::string> ret;
  fs::recursive_directory_iterator it(dir), eod;
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (p.extension() == ".h" || p.extension() == ".c") {
      std::ifstream ifs(p.string());
      assert(ifs.is_open());
      std::string line;
      while (std::getline(ifs, line)) {
        std::smatch match;
        if (std::regex_search(line, match, include_angle_reg)) {
          std::string file = match[1];
          if (Headers.count(file) == 0 || !header_exists(file)) {
            ret.insert(file);
          }
        }
      }
      ifs.close();
    }
  }
  return ret;
}
