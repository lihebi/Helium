#include "util/FileUtil.hpp"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <iostream>

namespace fs = boost::filesystem;

std::vector<std::string> FileUtil::GetFiles(const std::string& folder) {
  std::vector<std::string> vs;
  GetFiles(folder, vs);
  return vs;
}
// get file of the folder, store in vs
void FileUtil::GetFiles(const std::string& folder, std::vector<std::string>& vs) {
  fs::path project_folder(folder);
  fs::recursive_directory_iterator it(folder), eod;
  BOOST_FOREACH (fs::path const & p, std::make_pair(it, eod)) {
    if (is_regular_file(p)) {
      vs.push_back(p.string());
    }
  }
}

void FileUtil::GetFilesByExtension(
  const std::string& folder,
  std::vector<std::string>& vs,
  const std::string& extension
) {
  std::vector<std::string> ve {extension};
  GetFilesByExtension(folder, vs, ve);
}

void FileUtil::GetFilesByExtension(
  const std::string& folder,
  std::vector<std::string>& vs,
  const std::vector<std::string>& extension
) {
  fs::path project_folder(folder);
  fs::recursive_directory_iterator it(folder), eod;
  BOOST_FOREACH(fs::path const& p, std::make_pair(it, eod)) {
    if (is_regular_file(p)) {
      std::string with_dot = p.extension().string();
      // the extension may not exist
      if (!with_dot.empty()) {
        // if extension does not exist,
        // the substr(1) will cause exception of out of range of basic_string
        std::string without_dot = with_dot.substr(1);
        if (std::find(extension.begin(), extension.end(), with_dot) != extension.end()
        || std::find(extension.begin(), extension.end(), without_dot) != extension.end()) {
          vs.push_back(p.string());
        }
      }
    }
  }
}
