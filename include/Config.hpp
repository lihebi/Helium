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
  const std::string& GetFilename() const {return m_filename;}
  const std::string& GetOutputFolder() const {return m_output_folder;}
  const std::string& GetTmpFolder() const {return m_tmp_folder;}
  // segment
  const std::string& GetCodeSelectionMethod() const {return m_code_selection;}
  int GetMaxSegmentSize() const {return m_max_segment_size;}
  int GetSegmentTimeout() const {return m_segment_timeout;}
  // context
  const std::string& GetContextSearchMethod() const {return m_context_search;}
  int GetMaxLinearSearchValue() const {return m_max_linear_search_value;}
  int GetMaxContextSize() const {return m_max_context_size;}
  bool WillSimplifyBranch() const {return m_simplify_branch;}
  // build config
  const std::string& GetInstrumentPosition() const {return m_instrument_position;}
  const std::string& GetInstrumentType() const {return m_instrument_type;}
  int GetMaxSnippetSize() const {return m_max_snippet_size;}
  int GetMaxSnippetNumber() const {return m_max_snippet_number;}
  bool WillSimplifyOutputVar() const {return m_simplify_output_var;}

  bool WillBuildSaveCompilable() const {return m_build_save_compilable;}
  bool WillBuildSaveIncompilable() const {return m_build_save_incompilable;}
  // test config
  bool WillRunTest() const {return m_run_test;}
  const std::string& GetTestGenerationMethod() const {return m_test_generation;}
  int GetTestNumber() const {return m_test_number;}
  int GetTestTimeout() const {return m_test_timeout;}
  // analyzer config
  bool WillRunAnalyze() const {return m_run_analyze;}
  const std::string& GetAnalyer() const {return m_analyzer;}
  int GetAnalyzeTimeout() const {return m_analyze_timeout;}
  // debug
  bool WillShowCompileError() const {return m_show_compile_error;}
  int GetSkipSegment() const {return m_skip_segment;}
  // interact
  bool WillInteractReadSegment() const {return m_interact_read_segment;}
  bool WillInteractCompile() const {return m_interact_compile;}
  bool WillInteractCompileError() const {return m_interact_compile_error;}
  // cmd
  const std::string& GetCondCompMacros() const {return m_cond_comp_macros;}
  // output
  const std::string& GetOutputDefault() const {return m_output_default;}
  const std::string& GetOutputDebug() const {return m_output_debug;}
  const std::string& GetOutputTrace() const {return m_output_trace;}
  const std::string& GetOutputCompile() const {return m_output_compile;}
  const std::string& GetOutputData() const {return m_output_data;}
  const std::string& GetOutputRate() const {return m_output_rate;}
  const std::string& GetOutputTmp() const {return m_output_tmp;}
  const std::string& GetOutputWarning() const {return m_output_warning;}
  // mode
  const std::string& GetOutputDefaultMode() const {return m_output_default_mode;}
  const std::string& GetOutputDebugMode() const {return m_output_debug_mode;}
  const std::string& GetOutputTraceMode() const {return m_output_trace_mode;}
  const std::string& GetOutputCompileMode() const {return m_output_compile_mode;}
  const std::string& GetOutputDataMode() const {return m_output_data_mode;}
  const std::string& GetOutputRateMode() const {return m_output_rate_mode;}
  const std::string& GetOutputTmpMode() const {return m_output_tmp_mode;}
  const std::string& GetOutputWarningMode() const {return m_output_warning_mode;}
  // verbose
  bool IsLogTraceVerbose() const {return m_output_trace_verbose;}

private:
  Config() {}
  ~Config() {}
  static Config *m_instance;

  std::string m_filename;
  std::string m_output_folder;
  std::string m_tmp_folder;
  // segment
  std::string m_code_selection;
  int m_max_segment_size;
  int m_segment_timeout;
  // context
  std::string m_context_search;
  int m_max_linear_search_value;
  int m_max_context_size;
  bool m_simplify_branch;
  // build option
  std::string m_instrument_position;
  std::string m_instrument_type;
  int m_max_snippet_size;
  int m_max_snippet_number;
  bool m_simplify_output_var;

  bool m_build_save_compilable;
  bool m_build_save_incompilable;
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
  // output control option
  std::string m_output_default;
  std::string m_output_default_mode;
  std::string m_output_debug;
  std::string m_output_debug_mode;
  std::string m_output_trace;
  std::string m_output_trace_mode;

  bool m_output_trace_verbose;
  
  std::string m_output_compile;
  std::string m_output_compile_mode;
  std::string m_output_data;
  std::string m_output_data_mode;
  std::string m_output_rate;
  std::string m_output_rate_mode;
  std::string m_output_tmp;
  std::string m_output_tmp_mode;
  std::string m_output_warning;
  std::string m_output_warning_mode;
};

#endif
