#ifndef CSV_H
#define CSV_H

#include <string>
#include "common.h"

// class CSVRow {
// public:
//   std::string const &operator[](std::size_t index) const {
//     return m_data[index];
//   }
//   std::size_t size() const {
//     return m_data.size();
//   }
//   void readNextRow(std::istream& is) {
//     std::string line;
//     std::getline(is, line);
//     std::stringstream ss(line);
//     std::string cell;
//     while (std::getline(ss, cell, ',')) {
//       m_data.push_back(cell);
//     }
//   }
// private:
//   std::vector<std::string> m_data;
// };

class CSV {
public:
  CSV(std::istream &is, bool header_p);
  ~CSV() {}

  std::vector<std::string> Row(int idx) {
    return m_data[idx];
  }
  std::vector<std::string> Column(int idx) {
    if (idx >= ColumnNum() || idx < 0) return {};
    std::vector<std::string> ret;
    for (int i=0;i<(int)m_data.size();i++) {
      ret.push_back(m_data[i][idx]);
    }
    return ret;
  }
  std::vector<std::string> Column(std::string key) {
    if (m_map.count(key) == 0) return {};
    return m_map[key];
  }

  std::string get(std::string key, int row) {
    if (m_map.count(key) == 1) {
      if (row < (int)m_map[key].size() && row >= 0) {
        return m_map[key][row];
      }
    }
    return {};
  }
  std::string get(int row, int column) {
    return m_data[row][column];
  }

  bool HasHeader() {
    return !m_headers.empty();
  }
  std::vector<std::string> Headers() {
    return m_headers;
  }

  int RowNum() {
    return m_data.size();
  }
  int ColumnNum() {
    if (m_data.empty()) return 0;
    return m_data[0].size();
  }
  bool IsValid() {
    return m_valid;
  }
private:
  std::vector<std::string> m_headers;
  std::map<std::string, std::vector<std::string> > m_map;
  std::vector<std::vector<std::string> > m_data;
  bool m_valid = false;
};

class CSVFactory {
public:
  static CSV *CreateCSV(std::string file, bool header);
private:
};

#endif /* CSV_H */
