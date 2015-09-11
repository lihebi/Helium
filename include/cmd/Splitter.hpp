#ifndef __SPLITTER_HPP__
#define __SPLITTER_HPP__

#include <string>
#include <vector>

class Splitter {
public:
  Splitter(const std::string &folder);
  virtual ~Splitter();
  void Run();
private:
  std::string m_folder;
  std::vector<std::string> m_files;
  std::vector<std::string> m_stringcache;
};

#endif
