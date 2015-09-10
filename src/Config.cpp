#include <Config.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <set>
#include <exception>
#include <iostream>
namespace pt = boost::property_tree;

void Config::Load(const std::string& _filename) {
  std::cout<<"[Config] Loading Config: "<<_filename<<std::endl;
  filename = _filename;
  pt::ptree tree;
  pt::read_xml(filename, tree);
  // segment
  output_folder = tree.get("helium.output_folder", "helium_out");
  BOOST_FOREACH(pt::ptree::value_type &v, tree.get_child("helium.segment.excludes")) {
    excludes.insert(v.second.data());
  }
  code_selection = tree.get("helium.segment.code_selection", "loop");
  max_segment_size = tree.get("helium.segment.max_segment_size", 50);
  // context
  context_search = tree.get("helium.context.context_search", "linear");
  max_linear_search_value = tree.get("helium.context.max_linear_search_value", 0);
  // build option
  if (tree.get("helium.build.handle_struct", "false").compare("true") == 0) {
    handle_struct = true;
  } else {
    handle_struct = false;
  }
  if (tree.get("helium.build.handle_array", "false").compare("true") == 0) {
    handle_array = true;
  } else {
    handle_array = false;
  }
  BOOST_FOREACH(pt::ptree::value_type &v, tree.get_child("helium.build.instruments")) {
    _Instrument instrument;
    instrument.position = v.second.get<std::string>("position");
    instrument.type = v.second.get<std::string>("type");
    instruments.push_back(instrument);
  }
  // test
  if (tree.get("helium.test.run_test", "false").compare("true") == 0) {
    run_test = true;
  } else {
    run_test = false;
  }
  test_generation = tree.get("helium.test.test_generation", "random");
  test_number = tree.get("helium.test.test_number", 10);
  time_out = tree.get("helium.test.time_out", 100);
  // analyze
  if (tree.get("helium.analyze.run_analyze", "false").compare("true") == 0) {
    run_analyze = true;
  } else {
    run_analyze = false;
  }
  analyzer = tree.get("helium.analyze.analyzer", "invariant");
}
