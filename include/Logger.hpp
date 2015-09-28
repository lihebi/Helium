#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <string>
#include <fstream>

class Logger {
public:
  static Logger* Instance() {
    if (m_instance == 0) {
      m_instance = new Logger();
    }
    return m_instance;
  }
  void Log(const std::string& content);
private:
  Logger();
  ~Logger() {}
  static Logger* m_instance;
  std::ofstream m_logger;
};

#endif
