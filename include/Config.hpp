#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <iostream>
#include <vector>
#include <set>

struct Instrument {
  std::string position;
  std::string type;
};

class Config {
public:
  static Config* Instance();
  void Load(const std::string& filename);

  // General config
  const std::string& GetFilename() const {
    return m_filename;
  }
  const std::string& GetOutputFolder() const {
    return m_output_folder;
  }
  // build config
  const std::string& GetCodeSelectionMethod() const {
    return m_code_selection;
  }
  int GetMaxSegmentSize() const {
    return m_max_segment_size;
  }
  const std::string& GetContextSearchMethod() const {
    return m_context_search;
  }
  const int GetMaxLinearSearchValue() const {
    return m_max_linear_search_value;
  }
  const std::vector<Instrument>& GetInstruments() const {
    return m_instruments;
  }
  // test config
  bool WillRunTest() const {
    return m_run_test;
  }
  const std::string& GetTestGenerationMethod() const {
    return m_test_generation;
  }
  int GetTestNumber() const {
    return m_test_number;
  }
  int GetTestTimeout() const {
    return m_time_out;
  }
  // analyzer config
  bool WillRunAnalyze() const {
    return m_run_analyze;
  }
  const std::string& GetAnalyer() const {
    return m_analyzer;
  }
  // debug
  bool WillShowCompileError() const {return m_show_compile_error;}
  int GetSkipSegment() const {return m_skip_segment;}
  // interact
  bool WillInteractCompile() const {return m_interact_compile;}
  bool WillInteractCompileError() const {return m_interact_compile_error;}

private:
  Config() {}
  ~Config() {}
  static Config *m_instance;

  std::string m_filename;
  std::string m_output_folder;
  // segment
  std::string m_code_selection;
  int m_max_segment_size;
  // context
  std::string m_context_search;
  int m_max_linear_search_value;
  // build option
  std::vector<Instrument> m_instruments;
  // test
  bool m_run_test;
  std::string m_test_generation;
  int m_test_number;
  int m_time_out;
  // analyze
  bool m_run_analyze;
  std::string m_analyzer;
  // debug
  bool m_show_compile_error;
  int m_skip_segment;
  // interact
  bool m_interact_compile;
  bool m_interact_compile_error;
};

#endif
