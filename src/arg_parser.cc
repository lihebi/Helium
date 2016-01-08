#include "arg_parser.h"
#include <iostream>

ArgParser::ArgParser(int argc, char** argv)
{
  // create options_description
  po::options_description options("Arguments");
  // add options to it
  options.add_options()
    ("help,h", "produce help message") // --help, -h
    ("config,f", po::value<std::string>(), "config file")
    ("tagfile,c", po::value<std::string>(), "tag file")
    ("verbose,v", "verbose output")
    ("output,o", po::value<std::string>(), "output location")
    ;

  po::options_description util_options("Utils");
  util_options.add_options()
    ("create-tagfile", "create tag file")
    ("print-config", "print current config")
    ;
    

  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("folder", "project folder");
  // positional options: used for the option that don't need to use a --prefix
  po::positional_options_description positional;
  // this "folder" option include only one item
  positional.add("folder", 1);

  // Further group
  // all options avaliable for command line options
  m_cmdline_options
    .add(options)
    .add(util_options)
    .add(hidden)
    ;

  // this is the message that will show for help messages
  m_help_options
    .add(options)
    .add(util_options)
    ;

  /* run parser and store in m_vm */
  po::store(
            po::command_line_parser(argc, argv)
            .options(m_cmdline_options) // add cmdline options
            .positional(positional)     // add positional options
            .run(),                     // run the parser
            // store into m_vm
            m_vm
            );
  po::notify(m_vm);

  // /* validate the options */
  // if (!validate()) {
  //   PrintHelp();
  //   exit(1);
  // }
}

ArgParser::~ArgParser() {}

void ArgParser::PrintHelp() {
  std::cout<< "Usage: helium [options] <folder>" <<std::endl;
  std::cout<< m_help_options << std::endl;
}

bool ArgParser::Has(std::string name) {
  if (m_vm.count(name)) {
    return true;
  } else {
    return false;
  }
}

/*
 * Get content from vm
 * m_vm["folder"].as<std::string>();
 * m_vm["include-files"].as< std::vector<std::string> > ();
 */
std::string ArgParser::GetString(std::string name) {
  if (Has(name)) {
    return m_vm[name].as<std::string>();
  } else {
    return "";
  }
}

// bool ArgParser::validate() {
//   if (Has("folder")) return true;
//   else return false;
// }
