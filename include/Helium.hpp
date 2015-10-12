#ifndef __HELIUM_HPP__
#define __HELIUM_HPP__

#include <string>
#include <Config.hpp>
#include <vector>

class Helium {
public:
  Helium(const std::string &folder);
  virtual ~Helium();
private:
  std::string m_folder;
  std::vector<std::string> m_files;
};

#endif
