#ifndef __HELIUM_H__
#define __HELIUM_H__

#include "common.h"


class Helium {
public:
  Helium(int argc, char** argv);
  virtual ~Helium();
  void Run();
private:
  int countFunction();
  std::string m_folder;
  std::vector<std::string> m_files;
  std::string m_poi_file;
  // this requires m_benchmark to be set, to benchmark name and version combo
  // e.g. gzip-1.2.4
  std::string m_whole_poi;
  std::string m_benchmark;
};

#endif
