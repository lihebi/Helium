#include "Helium.hpp"
#include "segment/Segment.hpp"
#include "Reader.hpp"
#include "Builder.hpp"
#include "Tester.hpp"
#include "Analyzer.hpp"

#include "util/ThreadUtil.hpp"
#include "util/FileUtil.hpp"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <string>

namespace fs = boost::filesystem;

Helium::Helium(const std::string &folder)
: m_folder(folder) {
  std::cout<<"[Helium][Constructor]"<<std::endl;
  FileUtil::GetFilesByExtension(m_folder, m_files, "c");
  for (auto it=m_files.begin();it!=m_files.end();it++) {
    std::shared_ptr<Reader> reader = std::make_shared<Reader>(*it);
    reader->Read();
  }
}
Helium::~Helium() {}
