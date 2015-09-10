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
  void getFiles();
  void split(std::string line);
  void readfile(const std::string& filename);
  void writefile(const std::string& filename);
  std::string m_folder;
  std::vector<std::string> m_files;
  std::vector<std::string> m_stringcache;
};

#endif
