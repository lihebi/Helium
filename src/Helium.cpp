#include "Helium.hpp"
#include "segment/Segment.hpp"
#include "Reader.hpp"
#include "Builder.hpp"
#include "Tester.hpp"
#include "Analyzer.hpp"

#include "util/ThreadUtil.hpp"
#include "util/FileUtil.hpp"
#include "Logger.hpp"

#include "Global.hpp"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <string>

namespace fs = boost::filesystem;

Helium::Helium(const std::string &folder)
: m_folder(folder) {
  Logger::Instance()->LogTrace("[Helium][Constructor]\n");
  Logger::Instance()->LogAll("=====" __DATE__ __TIME__ + folder+" ======\n");
  FileUtil::GetFilesByExtension(m_folder, m_files, "c");
  Logger::Instance()->LogTrace(
                               "[Helium][Constructor] total number of c files: "
                               + std::to_string(m_files.size())
                               + "\n"
                               );
  for (auto it=m_files.begin();it!=m_files.end();it++) {
    global_file_error_number = 0;
    std::shared_ptr<Reader> reader = std::make_shared<Reader>(*it);
    reader->Read();
  }
}
Helium::~Helium() {}
