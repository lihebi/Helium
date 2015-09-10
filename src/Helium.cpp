#include <Helium.hpp>
#include <Segment.hpp>
#include <Reader.hpp>
#include <Builder.hpp>
#include <Tester.hpp>
#include <Analyzer.hpp>

#include "util/ThreadUtil.hpp"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <string>

namespace fs = boost::filesystem;

Helium::Helium(const std::string &folder, const Config &config)
: m_folder(folder), m_config(config) {
  std::cout<<"[Helium] Construct"<<std::endl;
  getFiles();
  // create reader
  for (auto it=m_files.begin();it!=m_files.end();it++) {
    std::shared_ptr<Reader> reader = std::make_shared<Reader>(*it, m_config);
  }
}
Helium::~Helium() {}

void Helium::getFiles() {
  fs::path project_folder(m_folder);
  fs::recursive_directory_iterator it(m_folder), eod;
  BOOST_FOREACH (fs::path const & p, std::make_pair(it, eod)) {
    if (is_regular_file(p)) {
      if (p.extension().string() == ".c") {
        if (m_config.excludes.count(p.filename().string()) == 0) {
          m_files.push_back(p.string());
        }
      }
    }
  }
}
