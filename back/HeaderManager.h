#ifndef HEADERMANAGER_H
#define HEADERMANAGER_H

#include <vector>
#include "helium/parser/AST.h"

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
  void addHeader(std::string s) {
    headers.insert(s);
    if (!fs::exists(s)) Exist=false;
  }
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
  std::string getPackage() {return package;}
  void addFlag(std::string flag) {flags.insert(flag);}
  /**
   * find if header is in the headers list.
   * @return whether it is in
   * @return include : the include path to be added
   */
  bool find(std::string header, std::set<std::string> &inc_flags);
  bool find(std::string header);
  std::set<std::string> getFlags() {
    std::set<std::string> ret;
    ret.insert(libs.begin(), libs.end());
    ret.insert(flags.begin(), flags.end());
    return ret;
  }
  bool exists() {return Exist;}

private:
  std::string package;
  std::set<std::string> headers;
  std::set<std::string> libs;
  std::set<std::string> flags;
  bool Exist=true;
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
  void dump(std::ostream &os);

  /**
   * replace pattern by "replace" in Deps
   */
  void adjustDeps(std::string pattern, std::string replace);

  void dumpDeps(std::ostream &os) {
    for (auto &m : Deps) {
      os << m.first << " ==> \n";
      for (auto s : m.second) {
        os << "\t" << s << "\n";
      }
    }
  }

  std::string dumpDepsInComment() {
    std::string ret;
    for (auto &m : Deps) {
      ret += "// " + m.first + " ==>\n";
      for (auto s : m.second) {
        ret += "// \t" + s + "\n";
      }
    }
    return ret;
  }

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
  void jsonTopoSortHeaders();

  bool jsonCheckHeader(std::string header);

  /**
   * system headers also has dependencies. For example,
   * #include <sys/socket.h> must be before #include <linux/wireless.h>
   */
  std::set<std::string> jsonGetHeaders() {return jsonHeaders;}
  std::set<std::string> jsonGetFlags() {return jsonFlags;}

  std::vector<std::string> jsonGetSortedHeaders() {return jsonSortedSystemHeaders;}

  void jsonAddValidIncludePath(std::string s) {JsonValidIncludePaths.insert(s);}
  bool jsonIncludePathValid(fs::path s) {
    if (JsonValidIncludePaths.count(s) == 1) return true;
    return false;
  }
  void jsonDump(std::ostream &os) {
    os << "Json Dump: " << "\n";
    for (HeaderConf conf : JsonConfs) {
      os << conf.getPackage() << " ";
    }
    os << "\n";
  }
  std::vector<std::string> getSortedHeaders() {return SortedHeaders;}

  /**
   * check if the benchmark is valid
   * - system header is supported
   */
  bool jsonValidBench(std::string &reason);
  bool jsonValidBench();

  std::set<std::string> jsonGetUnsupportedHeaders();
  

  
  
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
  std::vector<std::string> SortedHeaders;

  // json related
  std::vector<HeaderConf> JsonConfs;
  std::set<std::string> LocalIncludes;
  std::set<std::string> SystemIncludes;
  std::set<std::string> jsonHeaders;
  std::set<std::string> jsonFlags;

  // this is going to be populated during parse
  std::vector<std::string> jsonSortedSystemHeaders;

  std::set<std::string> unsupportedFakeLocalHeaders;

  std::set<fs::path> JsonValidIncludePaths;

};






#endif /* HEADERMANAGER_H */
