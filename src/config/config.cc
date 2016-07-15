#include "config.h"
#include "utils/utils.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include <sstream>

#include <gtest/gtest.h>

using namespace utils;

Config* Config::m_instance = 0;

Config::Config() {}
Config::~Config() {}

void
Config::ParseFile(std::string filename) {
  std::ifstream is;
  is.open(filename);
  if (is.is_open()) {
    parse(is);
  } else {
    std::cerr<<"Failed to parse config file: "<<filename<<"\n";
    exit(1);
  }
}


void
Config::ParseString(std::string s) {
  std::istringstream ss(s);
  parse(ss);
}

void
Config::parse(std::istream &is) {
  std::string line;
  while (getline(is, line)) {
    utils::trim(line);
    if (line.empty()) continue; // empty line
    if (line[0] == '#') continue; // the line starts from comment
    if (line.find('#') != std::string::npos) {
      // remove the post-line comment
      line = line.substr(0, line.find('#'));
    }
    assert(line.find('=') != std::string::npos);
    std::string key = line.substr(0, line.find('='));
    std::string value =line.substr(line.find('=')+1);
    trim(key);
    trim(value);
    assert(!key.empty());
    m_map[key] = value;
  }
}


std::string Config::ToString() {
  std::string result;
  std::vector<std::string> keys;
  for (auto item : m_map) {
    keys.push_back(item.first);
  }
  for (std::string key : keys) {
    result += key + " = " + m_map[key];
    result += "\n";
  }
  utils::trim(result);
  return result;
}

/*******************************
 ** Querying
 *******************************/

std::string Config::GetString(std::string name) {
  if (m_map.find(name) == m_map.end()) {
    std::cerr << "warning: cannot find key: " << name << " in config file.\n";
    return "";
  }
  return m_map[name];
}

bool Config::GetBool(std::string name) {
  if (m_map.find(name) == m_map.end()) {
    std::cerr << "error: cannot find key: " << name << " in config file.\n";
    assert(false);
  }
  std::string res = m_map[name];
  if (res == "true") return true;
  else if (res == "false") return false;
  else {
    std::cout << "error: " << name << " is not a boolean value in config file."  << "\n";
    assert(false);
  }
}

/**
 * -1 means something wrong. Should stop program.
 */
int Config::GetInt(std::string name) {
  if (m_map.find(name) == m_map.end()) {
    std::cerr << "warning: cannot find key: " << name << " in config file.\n";
    return -1;
  }
  // may throw
  // std::invalid_argument
  // std::out_of_range
  std::string s = m_map[name];
  // FIXME how to detect if config is not a number?
  // for (char c : s) {
  //   if (!isdigit(c)) return -1;
  // }
  // But no, may not! I have checked every char is digit!
  return std::stoi(m_map[name]);
}

/**
 * For all the keys in config file, check if it is also specified in args.
 * If yes, substitute the value.
 */
void Config::Overwrite(ArgParser &args) {
  std::string conf_str = args.GetString("conf");
  /**
   * The format of conf:
   * --conf="test-number=10, run-test=,xxx"
   */
  std::vector<std::string> pairs = utils::split(conf_str, ',');
  for (std::string &s : pairs) {
    utils::trim(s);
    std::vector<std::string> pair = utils::split(s, '=');
    std::string key, value;
    if (pair.size() == 1) {
      key = pair[0];
    } else if (pair.size() == 2) {
      key = pair[0];
      value = pair[1];
    }
    utils::trim(key);
    utils::trim(value);
    if (m_map.find(key) != m_map.end()) {
      m_map[key] = value;
    }
  }
}

TEST(config_test_case, config_test) {
  /**
   * Raw string must have prefix(optional) and braces(must)
   * Since it is raw string, don't write "\t", it will be literal!
   */
  const char *raw = R"raw_prefix(


aaa=hello
bbb=world
# this is comment
ccc-ddd  =  1200 # another comment


)raw_prefix";
  Config::Instance()->ParseString(std::string(raw));
  EXPECT_EQ(Config::Instance()->GetString("aaa"), "hello");
  EXPECT_EQ(Config::Instance()->GetString("bbb"), "world");
  EXPECT_EQ(Config::Instance()->GetString("ccc-ddd"), "1200");
  EXPECT_EQ(Config::Instance()->GetInt("ccc-ddd"), 1200);
  EXPECT_EQ(Config::Instance()->GetInt("bbb"), -1);
}
