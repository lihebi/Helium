#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <iostream>
#include <vector>
#include <set>

struct Config {
  void Load(const std::string& filename);
  void Save();
  struct _Instrument {
    std::string position;
    std::string type;
  };
  std::string filename;
  std::string output_folder;
  // segment
  std::set<std::string> excludes;
  std::string code_selection;
  int max_segment_size;
  // context
  std::string context_search;
  int max_linear_search_value;
  // build option
  bool handle_struct;
  bool handle_array;
  std::vector<_Instrument> instruments;
  // test
  bool run_test;
  std::string test_generation;
  int test_number;
  int time_out;
  // analyze
  bool run_analyze;
  std::string analyzer;
};

#endif
