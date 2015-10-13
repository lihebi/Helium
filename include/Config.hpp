#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <iostream>
#include <vector>
#include <set>

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
  // segment
  const std::string& GetCodeSelectionMethod() const {
    return m_code_selection;
  }
  int GetMaxSegmentSize() const {
    return m_max_segment_size;
  }
  int GetSegmentTimeout() const {return m_segment_timeout;}
  // context
  const std::string& GetContextSearchMethod() const {
    return m_context_search;
  }
  const int GetMaxLinearSearchValue() const {
    return m_max_linear_search_value;
  }
  // build config
  const std::string& GetInstrumentPosition() const {return m_instrument_position;}
  const std::string& GetInstrumentType() const {return m_instrument_type;}
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
    return m_test_timeout;
  }
  // analyzer config
  bool WillRunAnalyze() const {
    return m_run_analyze;
  }
  const std::string& GetAnalyer() const {
    return m_analyzer;
  }
  int GetAnalyzeTimeout() const {return m_analyze_timeout;}
  // debug
  bool WillShowCompileError() const {return m_show_compile_error;}
  int GetSkipSegment() const {return m_skip_segment;}
  // interact
  bool WillInteractReadSegment() const {return m_interact_read_segment;}
  bool WillInteractCompile() const {return m_interact_compile;}
  bool WillInteractCompileError() const {return m_interact_compile_error;}
  // cmd
  std::string GetCondCompMacros() const {return m_cond_comp_macros;}

private:
  Config() {}
  ~Config() {}
  static Config *m_instance;

  std::string m_filename;
  std::string m_output_folder;
  // segment
  std::string m_code_selection;
  int m_max_segment_size;
  int m_segment_timeout;
  // context
  std::string m_context_search;
  int m_max_linear_search_value;
  // build option
  std::string m_instrument_position;
  std::string m_instrument_type;
  // test
  bool m_run_test;
  std::string m_test_generation;
  int m_test_number;
  int m_test_timeout;
  // analyze
  bool m_run_analyze;
  std::string m_analyzer;
  int m_analyze_timeout;
  // debug
  bool m_show_compile_error;
  int m_skip_segment;
  // interact
  bool m_interact_read_segment;
  bool m_interact_compile;
  bool m_interact_compile_error;
  // cmd
  std::string m_cond_comp_macros;
};

#endif
