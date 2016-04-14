#include "slice_reader.h"
#include <fstream>
#include "utils.h"

SliceFile::SliceFile(std::string slicing_file) {
  std::ifstream is;
  is.open(slicing_file);
  assert(is.is_open());
  std::string line;
  getline(is, line);
  // criteria
  assert(line.find(':') != std::string::npos);
  m_criteria_linum = atoi(line.substr(line.find(':')+1).c_str());
  m_criteria_file = line.substr(0, line.find(':'));
  // fill slices
  while (std::getline(is, line)) {
    std::vector<std::string> v = utils::split(line);
    assert(v.size() == 2);
    std::string file = v[0];
    if (file.empty()) continue;
    // only accept relative path!
    // this is actually used to eliminate /usr/include staff
    if (file[0] == '/') continue;
    int linum = atoi(v[1].c_str());
    m_slices[file] = linum;
  }
}
