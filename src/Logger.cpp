#include "Logger.hpp"
#include <iostream>
#include "Config.hpp"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

Logger* Logger::m_instance = 0;

Logger::Logger() {
  m_logger.open("/tmp/helium_log.txt");
  if (!m_logger.is_open()) {
    std::cerr << "Logger open failed." << std::endl;
    exit(1);
  }
}


void
Logger::Log(const std::string& content) {
  std::cout << "[Logger::Log]" << std::endl;
  m_logger << content << std::endl;
  std::cout << content << std::endl;
  // exit(1);
}

void
Logger::Log(const std::string& filename, const std::string& content) {
  fs::path file_path(filename);
  fs::path dir = file_path.parent_path();
  if (!fs::exists(dir)) {
    fs::create_directories(dir);
  }
  std::ofstream os;
  os.open(filename, std::ios::app);
  if (os.is_open()) {
    os << content << std::endl;
  } else {
    std::cerr << "Unable to create log file: " << filename << std::endl;
    exit(1);
  }
}
