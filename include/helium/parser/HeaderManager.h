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

  /**
   * Whether header exists in system path
   */
  bool header_exists(const std::string header);
  void addConf(fs::path file);
  void dump(std::ostream &os);
  std::map<std::string, std::string> parseHeaderConf(fs::path file);
  std::set<std::string> getHeaders();
  std::set<std::string> getLibs();
  std::set<std::string> getIncludes() {return Includes;}

  /**
   * mask and only expose those used in the project
   * FIXME Do I need mask??
   */
  // void mask(fs::path p);
  // void unmask();

  /**
   * Do everything
   */
  void parseBench(fs::path dir);

  /**
   * replace pattern by "replace" in Deps
   */
  void adjustDeps(std::string pattern, std::string replace);
  std::map<std::string, std::set<std::string> > getDeps() {
    return Deps;
  }
  std::set<std::string> infoGetAllHeaders() {return all_headers;}
  std::set<std::string> infoGetAllMissedExistHeaders() {return all_missed_exist_headers;}
  std::set<std::string> infoGetAllMissedNonExistHeaders() {return all_missed_non_exist_headers;}
  
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
  std::set<std::string> ForceHeaders;
  std::map<std::string, std::string> Header2LibMap;
  std::set<std::string> NonExistHeaders;
  // std::set<std::string> Libs;
  std::set<std::string> Mask;
  static HeaderManager *instance;
  std::map<std::string, std::set<std::string> > Deps;


  // used for utils
  std::set<std::string> all_headers;
  std::set<std::string> all_missed_exist_headers;
  std::set<std::string> all_missed_non_exist_headers;

};


#endif /* HEADERMANAGER_H */
