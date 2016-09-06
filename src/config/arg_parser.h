#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

/**
 * Argument Parser.
 *
 * Design:
helium [options] <target-benchmark-folder>

[options]
-f, --config

 */
class ArgParser {
public:
  static ArgParser* Instance() {
    if (!m_instance) {
      m_instance = new ArgParser();
    }
    return m_instance;
  }
  void Set(int argc, char** argv);
  void PrintHelp();
  bool Has(std::string name);
  std::string GetString(std::string name);
  bool HasCmdUtils();
private:
  ArgParser() {}
  virtual ~ArgParser() {}
  
  bool validate();

  po::options_description m_cmdline_options;
  po::options_description m_help_options;
  po::variables_map m_vm;

  static ArgParser *m_instance;
};

#endif
