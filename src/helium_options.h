#ifndef HELIUM_OPTIONS_H
#define HELIUM_OPTIONS_H

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

class HeliumOptions {
public:
  static HeliumOptions* Instance() {
    if (!m_instance) {
      m_instance = new HeliumOptions();
    }
    return m_instance;
  }
  void ParseCommandLine(int argc, char** argv);
  void ParseConfigFile(std::string config_file);
  void PrintHelp();
  bool Has(std::string name) {
    if (m_vm.count(name)) {
      return true;
    } else {
      return false;
    }
  }

  /**
   * Get content from vm
   * m_vm["folder"].as<std::string>();
   * m_vm["include-files"].as< std::vector<std::string> > ();
   * CONFIRM What will be returned if the value is not a string? e.g. bool
   */
  std::string GetString(std::string key) {
    if (Has(key)) {
      return m_vm[key].as<std::string>();
    } else {
      return "";
    }
  }
  bool GetBool(std::string key) {
    if (Has(key)) {
      return m_vm[key].as<bool>();
    } else {
      return false;
    }
  }
  int GetInt(std::string key) {
    if (Has(key)) {
      return m_vm[key].as<int>();
    } else {
      return 0;
    }
  }


  
  bool HasCmdUtils();
  bool empty() {
    if (m_vm.empty()) return true;
    return false;
  }
private:
  HeliumOptions();
  virtual ~HeliumOptions() {}
  
  bool validate();

  po::options_description m_cmdline_options;
  po::options_description m_config_options;
  po::options_description m_help_options;
  po::positional_options_description m_positional;
  po::variables_map m_vm;

  static HeliumOptions *m_instance;
};


#endif /* HELIUM_OPTIONS_H */
