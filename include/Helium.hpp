#ifndef __HELIUM_HPP__
#define __HELIUM_HPP__

#include <string>
#include <Config.hpp>
#include <vector>

class Helium {
public:
  Helium(int argc, char** argv);
  virtual ~Helium();
  void Run();
private:
  std::string m_folder;
  std::vector<std::string> m_files;
};

#endif
