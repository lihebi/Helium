#include "Config.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <set>
#include <exception>
#include <iostream>
namespace pt = boost::property_tree;

Config *Config::m_instance = 0;
Config* Config::Instance() {
  if (m_instance == 0) {
    m_instance = new Config;
  }
  return m_instance;
}

void Config::Load(const std::string& filename) {
  std::cout<<"[Config] Loading Config: "<<filename<<std::endl;
  m_filename = filename;
  pt::ptree tree;
  pt::read_xml(m_filename, tree);
  // segment
  m_output_folder = tree.get("helium.output_folder", "helium_out");
  m_code_selection = tree.get("helium.segment.code_selection", "loop");
  m_max_segment_size = tree.get("helium.segment.max_segment_size", 50);
  // context
  m_context_search = tree.get("helium.context.context_search", "linear");
  m_max_linear_search_value = tree.get("helium.context.max_linear_search_value", 0);
  // build option
  BOOST_FOREACH(pt::ptree::value_type &v, tree.get_child("helium.build.instruments")) {
    Instrument instrument;
    instrument.position = v.second.get<std::string>("position");
    instrument.type = v.second.get<std::string>("type");
    m_instruments.push_back(instrument);
  }
  // test
  if (tree.get("helium.test.run_test", "false").compare("true") == 0) {
    m_run_test = true;
  } else {
    m_run_test = false;
  }
  m_test_generation = tree.get("helium.test.test_generation", "random");
  m_test_number = tree.get("helium.test.test_number", 10);
  m_time_out = tree.get("helium.test.time_out", 100);
  // analyze
  if (tree.get("helium.analyze.run_analyze", "false").compare("true") == 0) {
    m_run_analyze = true;
  } else {
    m_run_analyze = false;
  }
  m_analyzer = tree.get("helium.analyze.analyzer", "invariant");
  // debug
  if (tree.get("helium.debug.show_compile_error", "false").compare("true") == 0) {
    m_show_compile_error = true;
  } else {
    m_show_compile_error = false;
  }
  // interact
  if (tree.get("helium.interact.compile", "false").compare("true") == 0) {
    m_interact_compile = true;
  } else {
    m_interact_compile = false;
  }
  if (tree.get("helium.interact.compile_error", "false").compare("true") == 0) {
    m_interact_compile_error = true;
  } else {
    m_interact_compile_error = false;
  }
}
