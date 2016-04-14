#ifndef SLICE_READER_H
#define SLICE_READER_H

#include "common.h"

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


#endif /* SLICE_READER_H */
