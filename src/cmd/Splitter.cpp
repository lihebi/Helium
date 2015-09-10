#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include "cmd/Splitter.hpp"
#include "util/FileUtil.hpp"
#include "util/StringUtil.hpp"

namespace fs=boost::filesystem;

Splitter::Splitter(const std::string &folder) : m_folder(folder) {
  std::cout<<"[Splitter] Construct"<<std::endl;
}

Splitter::~Splitter() {}

void Splitter::Run() {
  getFiles();
  for (auto it=m_files.begin();it!=m_files.end();it++) {
    readfile(*it);
    writefile(*it);
  }
}
void Splitter::getFiles() {
  FileUtil::GetFilesByExtension(m_folder, m_files, "c");
}
bool is_variable_decl(const std::string& line) {
  return true;
}
bool contain_multiple_variable(const std::string& line) {
  return true;
}
bool need_split(const std::string& line) {
  if (is_variable_decl(line) && contain_multiple_variable(line)) {
    return true;
  }
  return false;
}

void get_type(const std::string& line) {}
void get_variable(const std::string& line) {}
void construct() {}
void Splitter::split(std::string line) {
  get_type(line);
  get_variable(line);
  construct();
}
void Splitter::readfile(const std::string &filename) {
  m_stringcache.clear();
  std::ifstream f;
  f.open(filename);
  std::string line;
  if (f.is_open()) {
    while(std::getline(f, line)) {
      StringUtil::trim(line);
      if (need_split(line)) {
        split(line);
      }
      m_stringcache.push_back(line);
    }
    f.close();
  }
}
void Splitter::writefile(const std::string& filename) {
  std::ofstream f;
  f.open(filename);
  if (f.is_open()) {
    for (auto s : m_stringcache) {
      f<<s;
      f<<std::endl;
    }
    f.close();
  }
}
