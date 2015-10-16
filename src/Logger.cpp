#include "Logger.hpp"
#include <iostream>
#include "Config.hpp"
#include <boost/filesystem.hpp>
#include <cstdio>

namespace fs = boost::filesystem;

Logger* Logger::m_instance = 0;

FILE*
get_logger(const std::string& prefix, const std::string& filename, int fd, const char* mode) {
  if (filename.empty()) {
    return fdopen(fd, "w");
  } else {
    std::string full_path = prefix + "/" + filename;
    fs::path p(full_path);
    fs::path dir = p.parent_path();
    if (!fs::exists(dir)) fs::create_directories(dir);
    FILE *fp = fopen(full_path.c_str(), mode);
    if (!fp) {
      perror("fopen");
      exit(1);
    }
    return fp;
  }
}

Logger::Logger() {
  m_log_folder = Config::Instance()->GetTmpFolder() + "/log";
  m_default_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputDefault(), 1,
    Config::Instance()->GetOutputDefaultMode().c_str()
  );
  m_debug_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputDebug(), 1,
    Config::Instance()->GetOutputDebugMode().c_str()
  );
  m_trace_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputTrace(), 1,
    Config::Instance()->GetOutputTraceMode().c_str()
  );
  m_compile_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputCompile(), 2,
    Config::Instance()->GetOutputCompileMode().c_str()
  );
  m_data_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputData(), 1,
    Config::Instance()->GetOutputDataMode().c_str()
  );
  m_rate_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputRate(), 1,
    Config::Instance()->GetOutputRateMode().c_str()
  );
  fputs("=======" __DATE__ __TIME__ "=======\n" , m_default_logger);
  fputs("=======" __DATE__ __TIME__ "=======\n" , m_debug_logger);
  fputs("=======" __DATE__ __TIME__ "=======\n" , m_trace_logger);
  fputs("=======" __DATE__ __TIME__ "=======\n" , m_compile_logger);
  fputs("=======" __DATE__ __TIME__ "=======\n" , m_data_logger);
  fputs("=======" __DATE__ __TIME__ "=======\n" , m_rate_logger);
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

void Logger::LogDebug(const std::string& content) {
  log(content.c_str(), m_debug_logger);
}
