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
  FILE *logger = getLogger(filename);
  fputs(content.c_str(), logger);
  fputc('\n', logger);
  fflush(logger);
}

FILE*
Logger::getLogger(const std::string& name) {
  if (m_loggers.find(name) != m_loggers.end()) {
    return m_loggers[name];
  } else {
    fs::path file_path(name);
    fs::path dir = file_path.parent_path();
    if (!fs::exists(dir)) {
      fs::create_directories(dir);
    }
    FILE *fp = fopen(name.c_str(), "w");
    if (fp) {
      m_loggers[name] = fp;
      return fp;
    } else {
      std::cerr << "Unable to create log file: " << name << std::endl;
      exit(1);
    }
  }
}
