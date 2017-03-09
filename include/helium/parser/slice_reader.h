#ifndef SLICE_READER_H
#define SLICE_READER_H

#include "helium/utils/common.h"

/**
 *
 
 A slice is specified by a file.
 The file contains the relative path filenames, with regard to the root of benchmark.
 E.g. the benchmark foo-project/some/path/to/a.c, in the slice, the filename will become "path/to/a.c"
 The relative filename allows to copy the slice amoung different machines.

The format of the slice is follows:
First line contains the filename the slicing is performed on, a colon, a line number
ALl following lines are the slice locations on the first line.

The slice location is specified one per line, containing
filename \t linum
 
 */
class SliceFile {
public:
  SliceFile(std::string slicing_file);
  ~SliceFile() {}
  std::string GetCriteriaFile() {return m_criteria_file;}
  int GetCriteriaLinum() {return m_criteria_linum;}
  std::map<std::string, int> GetSlices() {return m_slices;}
private:
  std::string m_criteria_file; // first line, where the slice is performed on
  int m_criteria_linum; // first line
  std::map<std::string, int> m_slices; // filename to linum mapping
};

/**
 * Use only relative path
 * It is simple in the sense that it only use the last component of the path, the filename
 */
class SimpleSlice {
public:
  static SimpleSlice *Instance() {
    if (!m_instance) {
      m_instance = new SimpleSlice();
    }
    return m_instance;
  }
  std::set<int> GetLineNumbers(std::string filename);
  bool IsValid() {
    return m_valid;
  }
  void SetSliceFile(std::string slice_file);
private:
  SimpleSlice() {}
  // a.c => {1, 5, 10}
  // b.c => {3, 18}
  std::map<std::string, std::set<int> > m_slice;
  static SimpleSlice *m_instance;
  bool m_valid = false;
};


#endif /* SLICE_READER_H */
