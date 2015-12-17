#include "Logger.hpp"
#include <iostream>
#include "Config.hpp"
#include <boost/filesystem.hpp>
#include <cstdio>

namespace fs = boost::filesystem;

Logger* Logger::m_instance = 0;

FILE*
get_logger(const std::string& prefix, const std::string& filename, const char* mode) {
  if (filename.empty()) {
    return NULL;
  } else {
    if (filename == "stdout") {
      return fdopen(STDOUT_FILENO, "w");
    } else if (filename == "stderr") {
      return fdopen(STDERR_FILENO, "w");
    }
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
    m_log_folder, Config::Instance()->GetOutputDefault(),
    Config::Instance()->GetOutputDefaultMode().c_str()
  );
  m_debug_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputDebug(),
    Config::Instance()->GetOutputDebugMode().c_str()
  );
  m_trace_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputTrace(),
    Config::Instance()->GetOutputTraceMode().c_str()
  );
  m_compile_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputCompile(),
    Config::Instance()->GetOutputCompileMode().c_str()
  );
  m_data_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputData(),
    Config::Instance()->GetOutputDataMode().c_str()
  );
  m_rate_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputRate(),
    Config::Instance()->GetOutputRateMode().c_str()
  );
  m_tmp_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputTmp(),
    Config::Instance()->GetOutputTmpMode().c_str()
  );
  m_warning_logger = get_logger(
    m_log_folder, Config::Instance()->GetOutputWarning(),
    Config::Instance()->GetOutputWarningMode().c_str()
  );
}

void log(const char* s, FILE *fp) {
  if (fp) {
    fputs(s, fp);
    fflush(fp);
  }
}


void
Logger::Log(const std::string& content) {
  log(content.c_str(), m_default_logger);
}

void
Logger::LogTrace(const std::string& content) {
  log(content.c_str(), m_trace_logger);
}

/*
 * Verbose trace logger.
 */ 
void
Logger::LogTraceV(const std::string& content) {
  if (Config::Instance()->IsLogTraceVerbose()) {
    log(content.c_str(), m_trace_logger);
  }
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

void Logger::LogTmp(const std::string& content) {
  log(content.c_str(), m_tmp_logger);
}

void Logger::LogWarning(const std::string& content) {
  log(content.c_str(), m_warning_logger);
}
void Logger::LogAll(const std::string& content) {
  Log(content);
  LogTrace(content);
  LogCompile(content);
  LogData(content);
  LogRate(content);
  LogDebug(content);
  LogTmp(content);
  LogWarning(content);
}
