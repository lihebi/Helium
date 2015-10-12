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
  void Log(const std::string& content);
  void Log(const std::string& filename, const std::string& content);
  void Logln(const std::string& filename, const std::string& content);
private:
  Logger();
  ~Logger() {}
  FILE* getLogger(const std::string& name);
  static Logger* m_instance;
  std::ofstream m_logger;
  std::map<std::string, FILE*> m_loggers;
};

#endif
