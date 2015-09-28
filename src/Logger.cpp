#include "Logger.hpp"
#include <iostream>
#include "Config.hpp"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

Logger* Logger::m_instance = 0;

Logger::Logger() {
  std::string folder = Config::Instance()->GetOutputFolder();
  fs::path file_path(folder + "/default_log.txt");
  fs::path dir = file_path.parent_path();
  if (!fs::exists(dir)) {
    fs::create_directories(dir);
  }
  m_logger.open(file_path.string());
  if (!m_logger.is_open()) {
    std::cerr << "Logger open failed." << std::endl;
    exit(1);
  }
}


void
Logger::Log(const std::string& content) {
  m_logger << content << std::endl;
}
