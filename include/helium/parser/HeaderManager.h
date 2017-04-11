#ifndef HEADERMANAGER_H
#define HEADERMANAGER_H

#include <vector>
#include "helium/parser/ast_v2.h"
#include "helium/utils/common.h"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <gtest/gtest.h>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

namespace fs = boost::filesystem;

#include <regex>


class HeaderConf {
public:
  HeaderConf() {}
  ~HeaderConf() {}
  void addHeader(std::string s) {headers.insert(s);}
  void addLib(std::string lib) {
    // for now, only consider /usr/lib/libxxx.so
    std::regex libreg("^/usr/lib/lib(.*)\\.so$");
    std::smatch match;
    if (std::regex_match(lib, match, libreg)) {
      std::string name = match[1].str();
      libs.insert("-l" + name);
    }
  }
  void setPackage(std::string s) {package = s;}
  void addFlag(std::string flag) {flags.insert(flag);}
  /**
   * find if header is in the headers list.
   * @return whether it is in
   * @return include : the include path to be added
   */
  bool find(std::string header, std::set<std::string> &inc_flags);
  std::set<std::string> getFlags() {
    std::set<std::string> ret;
    ret.insert(libs.begin(), libs.end());
    ret.insert(flags.begin(), flags.end());
    return ret;
  }
private:
  std::string package;
  std::set<std::string> headers;
  std::set<std::string> libs;
  std::set<std::string> flags;
  // std::set<std::string> flags;
  /**
   * TODO build a index from filename to actual file
   */
};

/**
 * Manager the headers on the system
 */
class HeaderManager {
public:
  HeaderManager() {}
  ~HeaderManager() {}
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


  /**
   * Add configure file
   */
  void jsonAddConf(fs::path file);
  /**
   * Parse the files in the bench to decide which header files to use
   */
  void jsonParseBench(fs::path bench);
  // only for test
  void jsonAddLocalInclude(std::string inc) {LocalIncludes.insert(inc);}
  // onlly for test
  void jsonAddSystemInclude(std::string inc) {SystemIncludes.insert(inc);}
  void jsonClearHeader() {
    LocalIncludes.clear();
    SystemIncludes.clear();
  }
  /**
   * For all the local and system includes, decide the
   * - header files
   * - include flags
   * - lib flags
   * Output
   * - jsonHeaders
   * - jsonFlags
   */
  void jsonResolve();

  std::set<std::string> jsonGetHeaders() {return jsonHeaders;}
  std::set<std::string> jsonGetFlags() {return jsonFlags;}
  
private:
  std::set<std::string> Includes = {
    "/usr/include/",
    "/usr/local/include/",
    "/usr/include/x86_64-linux-gnu/",
    "/usr/include/i386-linux-gnu/",
    "/usr/lib/gcc/x86_64-linux-gnu/6/include/",
    "/usr/lib/gcc/i386-linux-gnu/6/include/"
  };
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

  // json related
  std::vector<HeaderConf> JsonConfs;
  std::set<std::string> LocalIncludes;
  std::set<std::string> SystemIncludes;
  std::set<std::string> jsonHeaders;
  std::set<std::string> jsonFlags;

};


#endif /* HEADERMANAGER_H */
