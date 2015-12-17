#include "Config.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <set>
#include <exception>
#include <iostream>
#include <unistd.h>
namespace pt = boost::property_tree;

Config *Config::m_instance = 0;
Config* Config::Instance() {
  if (m_instance == 0) {
    m_instance = new Config;
  }
  return m_instance;
}

void Config::Load(const std::string& filename) {
  // std::cout<<"[Config::Load] "<<filename<<std::endl;
  m_filename = filename;
  pt::ptree tree;
  if (access(filename.c_str(), F_OK) == -1) {
    std::cerr << "[Config::Load] cannot load config file: " << filename << std::endl;
    exit(1);
  }
  pt::read_xml(m_filename, tree);
  // segment
  m_output_folder           = tree.get("helium.output_folder", "helium_out");
  m_tmp_folder              = tree.get("helium.tmp_folder", "/tmp/helium");
  m_code_selection          = tree.get("helium.segment.code_selection", "loop");
  m_max_segment_size        = tree.get("helium.segment.max_segment_size", 50);
  m_segment_timeout         = tree.get("helium.segment.timeout", 99999);
  // context
  m_context_search          = tree.get("helium.context.context_search", "linear");
  m_max_linear_search_value = tree.get("helium.context.max_linear_search_value", 0);
  m_max_context_size        = tree.get("helium.context.max_context_size", 99999);
  m_simplify_branch         = tree.get("helium.context.simplify_branch", "false").compare("true") == 0;
  // build option
  m_instrument_position     = tree.get("helium.build.instrument_position", "");
  m_instrument_type         = tree.get("helium.build.instrument_type", "");
  m_max_snippet_size        = tree.get("helium.build.max_snippet_size", 999999);
  m_max_snippet_number      = tree.get("helium.build.max_snippet_number", 99999);
  m_simplify_output_var     = tree.get("helium.build.simplify_output_var", "false").compare("true") == 0;
  m_build_save_compilable   = tree.get("helium.build.save_compilable", "false").compare("true") == 0;
  m_build_save_incompilable = tree.get("helium.build.save_incompilable", "false").compare("true") == 0;
  // test
  m_run_test                = tree.get("helium.test.run_test", "false").compare("true") == 0;
  m_test_generation         = tree.get("helium.test.test_generation", "random");
  m_test_number             = tree.get("helium.test.test_number", 10);
  m_test_timeout            = tree.get("helium.test.timeout", 99999);
  // analyze
  m_run_analyze             = tree.get("helium.analyze.run_analyze", "false").compare("true") == 0;
  m_analyzer                = tree.get("helium.analyze.analyzer", "invariant");
  m_analyze_timeout         = tree.get("helium.analyze.timeout", 99999);
  // debug
  m_show_compile_error      = tree.get("helium.debug.show_compile_error", "false").compare("true") == 0;
  m_skip_segment            = tree.get("helium.debug.skip_segment", 0);
  // interact
  m_interact_read_segment   = tree.get("helium.interact.read_segment", "false").compare("true") == 0;
  m_interact_compile        = tree.get("helium.interact.compile", "false").compare("true") == 0;
  m_interact_compile_error  = tree.get("helium.interact.compile_error", "false").compare("true") == 0;
  // cmd
  m_cond_comp_macros        = tree.get("helium.cmd.cond_comp_macros", "");
  // output
  m_output_default          = tree.get("helium.output.default.file", "");
  m_output_debug            = tree.get("helium.output.debug.file", "");
  m_output_trace            = tree.get("helium.output.trace.file", "");
  m_output_compile          = tree.get("helium.output.compile.file", "");
  m_output_data             = tree.get("helium.output.data.file", "");
  m_output_rate             = tree.get("helium.output.rate.file", "");
  m_output_tmp              = tree.get("helium.output.tmp.file", "");
  m_output_warning          = tree.get("helium.output.warning.file", "");
  // mode
  m_output_default_mode     = tree.get("helium.output.default.mode", "a");
  m_output_debug_mode       = tree.get("helium.output.debug.mode", "a");
  m_output_trace_mode       = tree.get("helium.output.trace.mode", "a");
  m_output_compile_mode     = tree.get("helium.output.compile.mode", "a");
  m_output_data_mode        = tree.get("helium.output.data.mode", "a");
  m_output_rate_mode        = tree.get("helium.output.rate.mode", "a");
  m_output_tmp_mode         = tree.get("helium.output.tmp.mode", "a");
  m_output_warning_mode     = tree.get("helium.output.tmp.mode", "a");
  // verbose
  m_output_trace_verbose    = tree.get("helium.output.trace.verbose", "false").compare("true") == 0;
}
