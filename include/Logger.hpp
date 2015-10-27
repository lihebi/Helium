#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <string>
#include <fstream>
#include <map>

class Logger {
public:
  static Logger* Instance() {
    if (m_instance == 0) {
      m_instance = new Logger();
    }
    return m_instance;
  }

  // void Log(const std::string& filename, const std::string& content);
  // void Logln(const std::string& filename, const std::string& content);
  // predefined logger
  void Log(const std::string& content);
  void LogTrace(const std::string& content);
  void LogCompile(const std::string& content);
  void LogData(const std::string& content);
  void LogRate(const std::string& content);
  void LogDebug(const std::string& content);
  void LogTmp(const std::string& content);
  void LogWarning(const std::string& content);
  void LogAll(const std::string& content);

  // log MyClass::MyMethod entries
  void LogTrace();
private:
  Logger();
  ~Logger() {}
  FILE* getLogger(const std::string& name);
  static Logger* m_instance;
  std::string m_log_folder;
  FILE* m_trace_logger;
  FILE* m_default_logger;
  FILE* m_debug_logger;
  FILE* m_compile_logger;
  FILE* m_data_logger;
  FILE* m_rate_logger;
  FILE* m_tmp_logger;
  FILE* m_warning_logger;
};

#endif
