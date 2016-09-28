#include "resolver.h"
#include "utils/utils.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include "helium_options.h"
#include "type/type_helper.h"

namespace fs = boost::filesystem;
using namespace utils;

SystemResolver* SystemResolver::m_instance = 0;

static bool header_exists(const std::string header) {
  fs::path p("/usr/include/"+header);
  if (fs::exists(p)) return true;
  p = "/usr/local/include/" + header;
  if (fs::exists(p)) return true;
  p = "/usr/include/x86_64-linux-gnu/" + header;
  if (fs::exists(p)) return true;
  return false;
}

std::map<std::string, std::string> parse_header_conf(std::string file) {
  std::ifstream is;
  is.open(file);
  if (!is.is_open()) {
    std::cerr << "EE: cannot open header conf file " << file << "\n";
    exit(1);
  }
  assert(is.is_open());
  std::string line;
  std::string flag;
  // from header name to compile flag
  std::map<std::string, std::string> headers;
  while (getline(is, line)) {
    trim(line);
    flag = "";
    if (!line.empty() && line[0] != '#') {
      if (line.find(' ') != std::string::npos) {
        flag = line.substr(line.find(' '));
        line = line.substr(0, line.find(' '));
        trim(flag);
      }
      headers[line] = flag;
    }
  }
  return headers;
}


/**
 * Parse the header config file.
 * Add to m_headers and m_libs
 */
void SystemResolver::parseHeaderConf(std::string file) {
  std::map<std::string, std::string> headers = parse_header_conf(file);
  for (auto m : headers) {
    if (header_exists(m.first)) {
      m_headers.insert(m.first);
      if (!m.second.empty()) {
        m_libs.insert(m.second);
      }
    }
  }
}


SystemResolver::SystemResolver() {
  std::string helium_home = HeliumOptions::Instance()->GetString("helium-home");
  parseHeaderConf(helium_home+"/etc/headers.conf.d/third-party.conf");
  parseHeaderConf(helium_home+"/etc/headers.conf.d/system.conf");
}

/**
 * Check headers in headers.conf exist or not.
 * Print out!
 */
void SystemResolver::check_headers() {
  std::string s = getenv("HELIUM_HOME");
  std::ifstream is;
  std::map<std::string, std::string> m1 = parse_header_conf(s+"/etc/headers.conf.d/system.conf");
  std::map<std::string, std::string> m2 = parse_header_conf(s+"/etc/headers.conf.d/third-party.conf");
  std::vector<std::string> system_suc;
  std::vector<std::string> system_err;
  std::vector<std::string> third_suc;
  std::vector<std::string> third_err;
  for (auto m: m1) {
    std::string header = m.first;
    if (header_exists(header)) {
      system_suc.push_back(header);
    } else {
      system_err.push_back(header);
    }
  }
  for (auto m: m2) {
    std::string header = m.first;
    if (header_exists(header)) {
      third_suc.push_back(header);
    } else {
      third_err.push_back(header);
    }
  }
  std::cout << "System Success Count: " << system_suc.size()  << "\n";
  for (std::string s : system_suc) {
    utils::print(s + " ", CK_Green);
  }
  std::cout  << "\n";
  std::cout << "System Error Count: " << system_err.size()  << "\n";
  for (std::string e : system_err) {
    utils::print(e + " ", CK_Red);
  }
  std::cout  << "\n";
  std::cout << "Third Party Success Count: " << third_suc.size()  << "\n";
  for (std::string e : third_suc) {
    utils::print(e + " ", CK_Green);
  }
  std::cout  << "\n";
  std::cout << "Third Party Error Count: " << third_err.size() << "\n";
  for (std::string e : third_err) {
    utils::print(e + " ", CK_Red);
  }
  std::cout  << "\n";
}

/**
 * DEPRECATED
 */
// std::string
// SystemResolver::GetHeaders() const {
//   std::string code;
//   // gnulib related
//   code +=
//     (Config::Instance()->GetBool("gnulib") ?
//      "#include <config.h>\n" // FIXME <> or "" ?
//      "#include <exclude.h>\n"
//      "#include <progname.h>\n"
//      "#include <gettext.h>\n"
//      "#include <error.h>\n"
//      "#include <dirname.h>\n"
//      : "")
//     // isdir does not need to include
//     ;
//   // other system files
//   for (auto it=m_headers.begin();it!=m_headers.end();it++) {
//     code += "#include <" + *it + ">\n";
//   }
//   return code;
// }

std::string
SystemResolver::GetLibs() const {
  std::string flags;
  for (auto it=m_libs.begin();it!=m_libs.end();it++) {
    flags += *it + " ";
  }
  return flags;
}

// resolve to primitive type
// uint8_t => unsigned char"
std::string
SystemResolver::ResolveType(const std::string& name) {
  std::vector<CtagsEntry> entries = Parse(name, "t");
  for (auto it=entries.begin();it!=entries.end();it++) {
    std::string pattern = it->GetPattern();
    // /^} FILE;$/
    if (pattern.find("typedef") == std::string::npos || pattern.rfind(';') == std::string::npos) continue;
    pattern = pattern.substr(pattern.find("typedef"));
    pattern = pattern.substr(0, pattern.rfind(';'));
    // FIXME typedef xxx xxx yyy ;
    std::vector<std::string> vs = split(pattern);
    // to_type is the middle part of split
    std::string to_type;
    for (size_t i=1;i<vs.size()-1;i++) {
      to_type += vs[i]+' ';
    }
    to_type.pop_back();
    // if it is primitive type, return
    if (is_primitive(to_type)) {
      return to_type;
    }
    // depth first resolve
    std::string new_type = ResolveType(to_type);
    if (!new_type.empty()) return new_type;
  }
  return "";
}

bool
SystemResolver::Has(const std::string& name) {
  // only consider type:
  //    g: enum
  //    s: struct
  //    u: union
  //    t: typedef
  std::vector<CtagsEntry> entries = Parse(name, "gsut");
  if (entries.empty()) return false;
  else {
    return true;
  }
}

void
SystemResolver::Load(const std::string& tagfile) {
  tagFileInfo *info = (tagFileInfo*)malloc(sizeof(tagFileInfo));
  m_tagfile = tagsOpen (tagfile.c_str(), info);
  if (info->status.opened != true) {
    std::cerr << "[SystemResolver::Load] cannot load tagfile: " << tagfile
    << ". Did you forget to make systype.tags file?" << std::endl;
    exit(1);
  }
  m_entry = (tagEntry*)malloc(sizeof(tagEntry));
}

std::vector<CtagsEntry>
SystemResolver::Parse(const std::string& name) {
  std::vector<CtagsEntry> vc;
  assert(m_tagfile != NULL);
  tagResult result = tagsFind(m_tagfile, m_entry, name.c_str(), TAG_FULLMATCH);
  while (result == TagSuccess) {
    if (m_entry->kind) {
      vc.push_back(CtagsEntry(m_entry));
    }
    // find next
    result = tagsFindNext(m_tagfile, m_entry);
  }
  return vc;
}


std::vector<CtagsEntry>
SystemResolver::Parse(const std::string& name, const std::string& type) {
  std::vector<CtagsEntry> vc;
  if (m_tagfile == NULL) {
    std::cerr << "SystemResolver::Parse m_tagfile is NULL. Load it first!"  << "\n";
    assert(false);
    return vc;
  }
  // assert(m_tagfile != NULL);
  tagResult result = tagsFind(m_tagfile, m_entry, name.c_str(), TAG_FULLMATCH);
  while (result == TagSuccess) {
    if (m_entry->kind && type.find(*(m_entry->kind)) != std::string::npos) {
      vc.push_back(CtagsEntry(m_entry));
    }
    // find next
    result = tagsFindNext(m_tagfile, m_entry);
  }
  return vc;
}
