#ifndef __HELIUM_HPP__
#define __HELIUM_HPP__

#include <string>
#include <Config.hpp>
#include <spdlog/spdlog.h>
#include <vector>

class Helium {
public:
  Helium(const std::string &folder, const Config &config);
  virtual ~Helium();
private:
  void getFiles();

  std::string m_folder;
  Config m_config;
  std::vector<std::string> m_files;

};

#endif
