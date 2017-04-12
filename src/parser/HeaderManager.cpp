#include "helium/parser/HeaderManager.h"

#include "helium/utils/string_utils.h"

#include "helium/resolver/SnippetV2.h"

#include "helium/utils/helium_options.h"
#include "helium/resolver/graph.h"

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
}

static std::regex include_reg("#\\s*include\\s*[\"<]([\\w/]+\\.h)[\">]");
static std::regex include_quote_reg("#\\s*include\\s*\"([\\w/]+\\.h)\"");
static std::regex include_angle_reg("#\\s*include\\s*<([\\w/]+\\.h)>");

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



TEST(HeaderManagerTest, MyTest) {
  HeaderManager manager;
  fs::path tmp_dir = utils::create_tmp_dir();

  const char *json = R"prefix(
  [{
    "package": "acl",
    "size": "135208",
    "includes": [
      "/usr/include/sys/acl.h",
      "/usr/include/acl/libacl.h"
    ],
    "libs": [
      "/usr/lib/libacl.so"
    ]
  }]
)prefix";

  fs::path conf_file = tmp_dir / "a.json";
  utils::write_file(conf_file, json);

  manager.jsonAddConf(conf_file);
  manager.jsonAddSystemInclude("sys/acl.h");
  manager.jsonAddSystemInclude("libacl.h");

  manager.jsonResolve();
  
  EXPECT_EQ(manager.jsonGetHeaders(), std::set<std::string>({"sys/acl.h", "libacl.h"}));
  EXPECT_EQ(manager.jsonGetFlags(), std::set<std::string>({"-lacl", "-I/usr/include/acl", "-I/usr/include"}));
}


void HeaderManager::jsonAddConf(fs::path file) {
  std::cout << "[HeaderManager::jsonAddConf] " << file << "\n";
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
            Deps[p.string()].insert(to);
          }
        }
        // angle <a.h>
        if (std::regex_search(line, match, include_angle_reg)) {
          std::string file = match[1];
          SystemIncludes.insert(file);
        }
      }
    }
  }
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
      graph.addEdge(to, from);
    }
  }
  SortedHeaders = graph.topoSort();
}
