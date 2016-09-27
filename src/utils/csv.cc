#include "csv.h"
#include "utils/utils.h"
#include <boost/filesystem.hpp>
#include <iostream>

namespace fs = boost::filesystem;

CSV *CSVFactory::CreateCSV(std::string file, bool header_p) {
  if (!fs::exists(file)) {
    std::cerr << "EE: csv file " << file << " does not exist." << "\n";
    return NULL;
  }
  std::ifstream ifs(file);
  if (!ifs.is_open()) {
    std::cerr << "EE: csv file " << file << " cannot be opened"  << "\n";
    return NULL;
  }
  CSV *ret = new CSV(ifs, header_p);
  ifs.close();
  if (ret->IsValid()) return ret;
  else return NULL;
}



CSV::CSV(std::istream &is, bool header_p) {
  int size = -1;
  if (header_p) {
    std::string header;
    std::getline(is, header);
    m_headers = utils::split(header, ',');
    size = m_headers.size();
    if (size == 0) {
      m_valid = false;
      return;
    }
  }
  std::string line;
  while (std::getline(is, line)) {
    std::vector<std::string> row = utils::split(line, ',');
    if (size == -1) {
      size = row.size();
    } else {
      if (size != (int)row.size()) {
        m_valid = false;
        return;
      }
    }
    m_data.push_back(row);
  }
  
  for (int i=0;i<(int)m_headers.size();i++) {
    for (int j=0;j<(int)m_data.size();j++) {
      m_map[m_headers[i]].push_back(m_data[i][j]);
    }
  }

  m_valid = true;
  return;
}
