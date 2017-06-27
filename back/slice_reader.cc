#include "helium/parser/slice_reader.h"
#include <fstream>
#include "helium/utils/utils.h"

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

SimpleSlice *SimpleSlice::m_instance = NULL;


/**
 * This file can be only lines with two columns
 * Do not include the cirteria line
 */
void SimpleSlice::SetSliceFile(std::string slice_file) {
  std::ifstream is;
  is.open(slice_file);
  assert(is.is_open());
  std::string line;
  while (std::getline(is, line)) {
    std::vector<std::string> v = utils::split(line);
    // simply disgard such lines
    if (v.size() != 2) continue;
    std::string file = v[0];
    file = utils::file_name_last_component(file);
    int linum = atoi(v[1].c_str());
    if (file.empty()) continue;
    m_slice[file].insert(linum);
  }
  m_valid = true;
}

/**
 * For each file, get the set of line numbers
 * It will change the filename to a simple format
 */
std::set<int> SimpleSlice::GetLineNumbers(std::string filename) {
  filename = utils::file_name_last_component(filename);
  if (m_slice.count(filename) == 1) {
    return m_slice[filename];
  } else {
    return {};
  }
}
