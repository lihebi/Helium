#include "Config.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include <sstream>

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
    trim(line);
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
