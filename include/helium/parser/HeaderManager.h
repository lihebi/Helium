#ifndef HEADERMANAGER_H
#define HEADERMANAGER_H

#include <vector>
#include "helium/parser/ast_v2.h"
#include "helium/utils/common.h"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <gtest/gtest.h>

namespace fs = boost::filesystem;

/**
 * Manager the headers on the system
 */
class HeaderManager {
public:
  static HeaderManager* Instance() {
    if (!instance) instance = new HeaderManager();
    return instance;
  }

  bool header_exists(const std::string header) {
    for (std::string s : Includes) {
      fs::path p(s + "/" + header);
      if (fs::exists(p)) return true;
    }
    return false;
  }

  void addConf(fs::path file) {
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

  /**
   * Discover headers used in the project dir, on current system, and
   * not in conf
   */
  std::set<std::string> discoverHeader(fs::path dir);
  /**
   * Discover headers used in project, but not on current system or
   * not in conf
   */
  std::set<std::string> checkHeader(fs::path dir);

  void dump(std::ostream &os) {
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
  /**
   * TODO Get headers
   */
  std::map<std::string, std::string> parseHeaderConf(fs::path file);

  std::set<std::string> getHeaders() {
    if (!masked) return Headers;
    std::set<std::string> ret;
    for (std::string s : Headers) {
      if (Mask.count(s) == 1) {
        ret.insert(s);
      }
    }
    return ret;
  }
  std::set<std::string> getLibs() {
    std::set<std::string> headers = getHeaders();
    std::set<std::string> ret;
    for (std::string h : headers) {
      if (Header2LibMap.count(h) == 1) {
        ret.insert(Header2LibMap[h]);
      }
    }
    return ret;
  }
  std::set<std::string> getIncludes() {return Includes;}

  /**
   * mask and only expose those used in the project
   */
  void mask(fs::path p);
  void unmask();
  
private:
  std::set<std::string> Includes = {
    "/usr/include/",
    "/usr/local/include/",
    "/usr/include/x86_64-linux-gnu/",
    "/usr/include/i386-linux-gnu/",
    "/usr/lib/gcc/x86_64-linux-gnu/6/include/",
    "/usr/lib/gcc/i386-linux-gnu/6/include/"
  };
  HeaderManager() {}
  ~HeaderManager() {}
  std::set<std::string> Headers;
  std::map<std::string, std::string> Header2LibMap;
  std::set<std::string> NonExistHeaders;
  // std::set<std::string> Libs;
  std::set<std::string> Mask;
  bool masked = false;
  static HeaderManager *instance;

};


#endif /* HEADERMANAGER_H */
