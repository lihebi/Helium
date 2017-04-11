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
      // std::set<std::string> disabled_packages = {"libbsd", "aarch64-linux-gnu-gcc", "dietlibc"};
      std::string package = v["package"].GetString();
      if (JsonDisabledPackages.count(package) == 1) continue;
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
    JsonConfs.push_back(conf);
  }
}

void HeaderManager::jsonParseBench(fs::path bench) {
  fs::recursive_directory_iterator it(bench), eod;
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
  // system includes
  for (std::string header : SystemIncludes) {
    for (HeaderConf &conf : JsonConfs) {
      std::set<std::string> inc_flags;
      if (conf.find(header, inc_flags)) {
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
