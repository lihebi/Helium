#ifndef __ARG_PARSER_HPP__
#define __ARG_PARSER_HPP__

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

class ArgParser {
public:
  ArgParser(int argc, char** argv);
  virtual ~ArgParser();
  void PrintHelp();
  bool Has(std::string name);
  std::string GetString(std::string name);
  bool HasCmdUtils();
private:
  bool validate();

  po::options_description m_cmdline_options;
  po::options_description m_help_options;

  po::variables_map m_vm;
};

#endif
