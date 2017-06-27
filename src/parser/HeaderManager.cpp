#include "helium/parser/HeaderManager.h"

#include "helium/utils/StringUtils.h"

#include "helium/resolver/SnippetV2.h"

#include "helium/utils/helium_options.h"
#include "helium/utils/graph.h"

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
    // FIXME suffix is not enough. it should be path suffix
    // if (has_suffix(s, sub)) {
    //   ret.insert(s);
    // }
    // I'm using path
    if (utils::match_suffix(s, sub)) {
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

void HeaderManager::dump(std::ostream &os) {
  os << "[HeaderManager] Headers:\n";
  os << "[HeaderManager] Exist Headers: ";
  for (HeaderConf conf : JsonConfs) {
    if (conf.exists()) {
      os << conf.getPackage() << " ";
    }
  }
  os << "\n";
  os << "[HeaderManager] Non Exist Headers: ";
  for (HeaderConf conf : JsonConfs) {
    if (!conf.exists()) {
      os << conf.getPackage() << " ";
    }
  }
  os << "\n";
  os << "[HeaderManager] Used Local Headers: ";
  for (std::string s : LocalIncludes) {
    os << s << " ";
  }
  os << "\n";
  
  os << "[HeaderManager] Used System Headers: ";
  for (std::string s : SystemIncludes) {
    os << s << " ";
  }
  os << "\n";
}

// FIXME adding - for filename, fix this for other regular expressions
static std::regex include_reg("#\\s*include\\s*[\"<]([\\w/-]+\\.h)[\">]");
static std::regex include_quote_reg("#\\s*include\\s*\"([\\w/-]+\\.h)\"");
static std::regex include_angle_reg("#\\s*include\\s*<([\\w/-]+\\.h)>");



TEST(HeaderManagerTest, RegexTest) {
  {
    std::smatch match;
    std::string line = "#include <hello.h>";
    bool b = std::regex_search(line, match, include_angle_reg);
    ASSERT_TRUE(b);
    EXPECT_EQ(match[1], "hello.h");
  }
  {
    std::smatch match;
    std::string line = "#include <foo/hello.h>";
    bool b = std::regex_search(line, match, include_angle_reg);
    ASSERT_TRUE(b);
    EXPECT_EQ(match[1], "foo/hello.h");
  }
  {
    std::smatch match;
    std::string line = "#include <llvm-c/Core.h>";
    bool b = std::regex_search(line, match, include_angle_reg);
    ASSERT_TRUE(b);
    EXPECT_EQ(match[1], "llvm-c/Core.h");
  }
  {
    std::smatch match;
    std::string line = "#include \"hello.h\"";
    bool b = std::regex_search(line, match, include_quote_reg);
    ASSERT_TRUE(b);
    EXPECT_EQ(match[1], "hello.h");
  }
}



void HeaderManager::adjustDeps(std::string pattern, std::string replace) {
  std::map<std::string, std::set<std::string> > tmp;
  for (auto &m : Deps) {
    for (std::string to : m.second) {
      std::string from = m.first;
      assert(from.find(pattern) == 0);
      assert(to.find(pattern) == 0);
      from = from.replace(0, pattern.length(), replace);
      to = to.replace(0, pattern.length(), replace);
      tmp[from].insert(to);
    }
  }
  Deps = tmp;
}

/**
 * I need to add some restrictions
 * - /usr/include/bits should not be the includ evalue
 * - /usr/include/sys should not be the -I value, it redefines many headers like fcntl.h, signal.h
 *   The headers inside it is used as sys/ctypes.h.
 */
bool HeaderConf::find(std::string header, std::set<std::string> &ret) {
  std::set<fs::path> disabled = {"", ""};
  for (const std::string &s : headers) {
    if (utils::match_suffix(s, header)) {
      fs::path tmp = utils::substract_suffix(s, header);
      // filter out some restrictions
      // if (!HeaderManager::Instance()->jsonIncludePathDisabled(tmp)) {
      //   ret.insert("-I" + tmp.string());
      // }
      if (HeaderManager::Instance()->jsonIncludePathValid(tmp)) {
        ret.insert("-I" + tmp.string());
      }
    }
  }
  if (ret.size() > 0) return true;
  return false;
}
bool HeaderConf::find(std::string header) {
  std::set<std::string> ret;
  return find(header, ret);
}



// TEST(HeaderManagerTest, MyTest) {
//   HeaderManager manager;
//   fs::path tmp_dir = utils::create_tmp_dir();

//   const char *json = R"prefix(
//   [{
//     "package": "acl",
//     "size": "135208",
//     "includes": [
//       "/usr/include/sys/acl.h",
//       "/usr/include/acl/libacl.h"
//     ],
//     "libs": [
//       "/usr/lib/libacl.so"
//     ]
//   }]
// )prefix";

//   fs::path conf_file = tmp_dir / "a.json";
//   utils::write_file(conf_file, json);

//   manager.jsonAddConf(conf_file);
//   manager.jsonAddSystemInclude("sys/acl.h");
//   manager.jsonAddSystemInclude("libacl.h");

//   manager.jsonResolve();
  
//   EXPECT_EQ(manager.jsonGetHeaders(), std::set<std::string>({"sys/acl.h", "libacl.h"}));
//   EXPECT_EQ(manager.jsonGetFlags(), std::set<std::string>({"-lacl", "-I/usr/include/acl", "-I/usr/include"}));
// }


void HeaderManager::jsonAddConf(fs::path file) {
  // std::cout << "[HeaderManager::jsonAddConf] " << file << "\n";
  rapidjson::Document document;
  std::ifstream ifs(file.string());
  rapidjson::IStreamWrapper isw(ifs);
  document.ParseStream(isw);
  assert(document.IsArray());

  // load into internal representation
  // linux-atm
  for (rapidjson::Value &v : document.GetArray()) {
    HeaderConf conf;
    assert(v.IsObject());
    if (v.HasMember("package")) {
      std::string package = v["package"].GetString();
      conf.setPackage(package);
    }
    if (v.HasMember("includes")) {
      for (rapidjson::Value &header : v["includes"].GetArray()) {
        conf.addHeader(header.GetString());
      }
    }
    if (v.HasMember("libs")) {
      for (rapidjson::Value &lib : v["libs"].GetArray()) {
        conf.addLib(lib.GetString());
      }
    }
    if (v.HasMember("flags")) {
      for (rapidjson::Value &flag : v["flags"].GetArray()) {
        conf.addFlag(flag.GetString());
      }
    }
    JsonConfs.push_back(conf);
  }
}

void HeaderManager::jsonParseBench(fs::path bench) {
  // std::cout << "HeaderManager::jsonParseBench" << "\n";
  fs::recursive_directory_iterator it(bench), eod;
  std::set<std::string> all_files;
  {
    // get all files
    fs::recursive_directory_iterator it(bench), eod;
    BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
      if (p.extension() == ".h" || p.extension() == ".c") {
        all_files.insert(p.string());
      }
    }
  }
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
          LocalIncludes.insert(file);
          std::set<std::string> matches = filter_suffix(all_files, file);
          for (std::string to : matches) {
            // both first and seconds are ABSOLUTE path
            // std::cout << "parsebench: adding dep " << p.string() << " => " << to  << "\n";
            // FIXME this match might match a comment HACK to avoid a
            // DAG problem for graph toposort, the header including
            // itself is omitted. This typically should not happen,
            // but it can happen when the #include directive is in in
            // comment
            if (p.string() != to) {
              Deps[p.string()].insert(to);
            }
          }
          // this local include is not found in local files
          // check and adjust local includes and system includes
          // try to find the local include in all_files to verify if it is really a local file
          // - if find, do nothing to it
          // - otherwise, we need to check if it is a supported 3rd party library
          //   - if yes, add it to the system include
          //   - if no, print error? Ignore it
          if (matches.empty()) {
            bool supported = jsonCheckHeader(file);
            if (supported) {
              jsonSortedSystemHeaders.push_back(file);
              SystemIncludes.insert(file);
            } else {
              // save in class variable
              unsupportedFakeLocalHeaders.insert(file);
            }
          }
        }
        // angle <a.h>
        if (std::regex_search(line, match, include_angle_reg)) {
          std::string file = match[1];
          // std::cout << file << "\n";
          SystemIncludes.insert(file);
          jsonSortedSystemHeaders.push_back(file);
        }
      }
    }
  }
}

std::set<std::string> HeaderManager::jsonGetUnsupportedHeaders() {
  std::set<std::string> ret;
  for (std::string header : SystemIncludes) {
    bool found = false;
    for (HeaderConf &conf : JsonConfs) {
      if (conf.find(header)) {
        found = true;
        break;
      }
    }
    if (!found) {
      ret.insert(header);
    }
  }
  return ret;
}

bool HeaderManager::jsonValidBench(std::string &reason) {
  if (!unsupportedFakeLocalHeaders.empty()) {
    reason = "unsupported fake local headers: ";
    for (std::string header : unsupportedFakeLocalHeaders) {
      reason += header + " ";
    }
    return false;
  }
  // check for other system headers
  std::set<std::string> unsupported = jsonGetUnsupportedHeaders();
  reason = "unsupported system headers: ";
  for (std::string s : unsupported) {
    reason += s + " ";
  }
  return unsupported.empty();
}

bool HeaderManager::jsonValidBench() {
  std::string tmp;
  return jsonValidBench(tmp);
}


void HeaderManager::jsonResolve() {
  // HACK adding all the valid path to the flags
  if (HeliumOptions::Instance()->Has("header-valid-include-paths")) {
    std::vector<std::string> v = HeliumOptions::Instance()->GetStringVector("header-valid-include-paths");
    for (std::string s : v) {
      jsonFlags.insert("-I" + s);
    }
  }
  // if the header is used in <>
  // find if it is captured in the config file
  // if yes, include it
  for (std::string header : SystemIncludes) {
    for (HeaderConf &conf : JsonConfs) {
      std::set<std::string> inc_flags;
      if (conf.find(header, inc_flags)) {
        if (!conf.exists()) {
          std::cerr << "[HeaderManager] [Warning] package " << conf.getPackage()
                    << " does not exist, but it is included with <" << header << ">" << "\n";
        }
        jsonHeaders.insert(header);
        for (std::string inc : inc_flags) {
          jsonFlags.insert(inc);
        }
        std::set<std::string> flags = conf.getFlags();
        jsonFlags.insert(flags.begin(), flags.end());
      }
    }
  }

  std::set<std::string> done;
  std::vector<std::string> tmp;
  for (std::string header : jsonSortedSystemHeaders) {
    if (done.count(header)==1) continue;
    done.insert(header);
    for (HeaderConf &conf : JsonConfs) {
      if (conf.find(header)) {
        tmp.push_back(header);
      }
    }
  }
  jsonSortedSystemHeaders = tmp;
}


/**
 * Check whether the header is captured by the json config files.
 */
bool HeaderManager::jsonCheckHeader(std::string header) {
  // std::set<std::string> ret;
  for (HeaderConf &conf : JsonConfs) {
    if (conf.find(header)) {
      return true;
      // ret.insert(conf.getPackage());
    }
  }
  return false;
  // return ret;
}

// std::map<std::string, std::set<std::string> > Deps;
// std::vector<std::string> SortedHeaders;
void HeaderManager::jsonTopoSortHeaders() {
  SortedHeaders.clear();
  hebigraph::Graph<std::string> graph;
  // FIXME if a file does not depend on others, it should also be added to the graph
  for (auto &m : Deps) {
    graph.addNode(m.first);
  }
  for (auto &m : Deps) {
    std::string from = m.first;
    if (!graph.hasNode(from)) graph.addNode(from);
    for (std::string to : m.second) {
      // arrow means "used by", while FileDep means "depend on",
      // they are opposite, reflected here
      if (!graph.hasNode(to)) graph.addNode(to);
      // std::cout << "adding edge " << to << " => " << from << "\n";
      graph.addEdge(to, from);
    }
  }
  // graph node size
  // graph.dump(std::cout);
  // graph.dump(std::cout);
  SortedHeaders = graph.topoSort();
  // sorted header size
  // std::cout << "Sorted Header size: " << SortedHeaders.size() << "\n";
}
