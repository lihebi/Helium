#include "helium/utils/Helper.h"
#include "helium/utils/Common.h"
#include "helium/utils/Log.h"
#include "helium/utils/Utils.h"
#include <iostream>

#include "helium/utils/Utils.h"

/**
 * Get true linum in consideration of line marker
 * @param filename the path to the "cpped" file
 * @param linum the linum of the original file before preprocessing
 * @return the linum inside the "cpped" file that is the same as the line in original file
 */
int get_true_linum(std::string filename, int linum) {
  helium_print_trace("get_true_linum");
  // std::cout << filename << ':' << linum  << "\n";
  std::string content = utils::read_file(filename);
  std::vector<std::string> sp = utils::split(content, '\n');
  int ret = linum;
  for (int idx=0;idx<(int)sp.size();idx++) {
    std::string line = sp[idx];
    if (line.length() > 0 && line[0] == '#') {
      std::vector<std::string> line_marker = utils::split(line);
      if (line_marker.size() > 2) {
        std::string linum_str = line_marker[1];
        assert(utils::is_number(linum_str));
        int marker_linum = std::stoi(linum_str);
        if (marker_linum <= linum) {
          // update ret
          ret = idx + linum - marker_linum + 2;
          // std::cout << "marker: " << marker_linum << " linum: " << linum << " current: " << idx  << "\n";
        } else {
          return ret;
        }
      }
    }
  }
  return ret;
}


/**
 * @param folder the folder containing the source files and executables
 * @param src_file source file name, e.g. main.c
 */
Gcov::Gcov(fs::path folder, fs::path src_file)
  : m_folder(folder), m_src_file(src_file) {
  std::string cmd = "cd " + m_folder.string() + " && gcov -b " + m_src_file.string();
  /**
   * Lines executed:94.12% of 17                                                                        │
   * Branches executed:100.00% of 2                                                                     │
   * Taken at least once:50.00% of 2                                                                    │
   * Calls executed:100.00% of 10
   */
  std::string stdout = utils::new_exec(cmd.c_str());
  std::vector<std::string> lines = utils::split(stdout, '\n');
  std::string branch_prefix = "Taken at least once:";
  std::string stmt_prefix = "Lines executed:";
  for (std::string line : lines) {
    if (line.find(branch_prefix) == 0) {
      m_branch_cov = line.substr(branch_prefix.length());
    }
    if (line.find(stmt_prefix) == 0) {
      m_stmt_cov = line.substr(stmt_prefix.length());
    }
  }
  // if (m_branch_cov.empty() || m_stmt_cov.empty()) {
  //   std::cerr << "WW: coverage information empty. Stdout:" << "\n";
  //   std::cerr << stdout << "\n";
  // }
}
