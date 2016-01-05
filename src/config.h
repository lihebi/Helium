#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <map>

#include "arg_parser.h"

class Config {
public:
  static Config* Instance() {
    if (m_instance == NULL) {
      m_instance = new Config();
    }
    return m_instance;
  }
  ~Config();
  void ParseFile(std::string filename);
  void ParseString(std::string s);
  std::string GetString(std::string name);
  int GetInt(std::string name);
  void Overwrite(ArgParser &args);
private:
  Config();
  void parse(std::istream& is);
  std::map<std::string, std::string> m_map;
  static Config* m_instance;
};

#endif
