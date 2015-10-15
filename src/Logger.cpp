#include "Logger.hpp"
#include <iostream>
#include "Config.hpp"
#include <boost/filesystem.hpp>
#include <cstdio>

namespace fs = boost::filesystem;

Logger* Logger::m_instance = 0;

Logger::Logger() {
  m_log_folder = Config::Instance()->GetTmpFolder() + "/log";
  std::string default_config = Config::Instance()->GetOutputDefault();
  if (default_config.empty()) {
    m_default_logger = fdopen(1, "w");
  } else {
    m_default_logger = getLogger(m_log_folder + "/" + default_config);
  }
  std::string trace_config = Config::Instance()->GetOutputTrace();
  if (trace_config.empty()) {
    m_trace_logger = fdopen(1, "w");
  } else {
    m_trace_logger = getLogger(m_log_folder + "/" + trace_config);
  }
  std::string compile_config = Config::Instance()->GetOutputCompile();
  if (compile_config.empty()) {
    m_compile_logger = fdopen(2, "w");
  } else {
    m_compile_logger = getLogger(m_log_folder + "/" + compile_config);
  }
}

void log(const char* s, FILE *fp) {
  fputs(s, fp);
  fflush(fp);
}


void
Logger::Log(const std::string& content) {
  log(content.c_str(), m_default_logger);
}

void
Logger::LogTrace(const std::string& content) {
  log(content.c_str(), m_trace_logger);
}

void
Logger::LogCompile(const std::string& content) {
  log(content.c_str(), m_compile_logger);
}

void
Logger::LogData(const std::string& content) {
  log(content.c_str(), m_data_logger);
}
void
Logger::LogRate(const std::string& content) {
  log(content.c_str(), m_rate_logger);
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
