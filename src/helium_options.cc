#include "helium_options.h"
#include "utils/utils.h"
#include <iostream>
#include <fstream>

#include <gtest/gtest.h>

HeliumOptions *HeliumOptions::m_instance = NULL;

/*******************************
 ** HeliumOptions
 *******************************/

HeliumOptions::HeliumOptions() {
  // create options_description
  po::options_description general_options("Arguments");
  // add options to it
  general_options.add_options()
    ("help,h", "produce help message") // --help, -h
    ("verbose,v", "verbose mode")
    ("config,f", po::value<std::string>(), "config file")
    ("poi", po::value<std::string>(), "poi file")
    // The following two must be together
    ("whole-poi", po::value<std::string>(), "whole poi")
    ("benchmark,b", po::value<std::string>(), "benchmark name (including version, e.g. gzip-1.2.4, libgd_3_0_33)")
    ("tagfile,t", po::value<std::string>(), "tag file")
    ("snippet-db-folder,s", po::value<std::string>(), "snippet database folder")
    ("src-folder,c", po::value<std::string>(), "source file folder (not orig)")
    ("output,o", po::value<std::string>(), "output location")
    ("print,p", po::value<std::string>()->implicit_value(""), "what to be print")
    ("debug,d", po::value<std::string>()->implicit_value(""), "debugging pause point")
    ("slice", po::value<std::string>(),
     "slice to use, as the code selection. THIS WILL SET THE CODE SELECTION METHOD TO SLICE")
    ("slice-file", po::value<std::string>(), "the slice file, this will be used as a mask on the AST")
    ;

  po::options_description util_options("Utils");
  util_options.add_options()
    ("create-tagfile", "create tag file")
    ("create-snippet-db", "create snippet database")
    ("create-header-dep", "create header dependence table")
    ("print-header-dep", "the new dep printing: from the database")
    ("print-callgraph", "print callgraph")
    // ("create-srcml", "create xml from C source file")
    ("print-config", "print current config")
    ("print-segments", "print segments and exit")
    ("print-segment-info", "print segment count, segment size LOC in total, for every file")
    ("print-header-deps", "print header dependencies") // DEPRECATED
    ("check-headers", "check if the headers in headers.conf exists on this machine")
    ("create-function-ast", "create ast for all the functions in the target benchmarks")
    ("print-headers", "print header")
    ("print-meta", "print meta data")
    ("resolve-system-type", "Resolve a system type and print out result")
    ("print-cfg", "print CFG for each function")
    ("print-ast", "print AST for each function")
    ;

  po::options_description print_options("Print Options");
  print_options.add_options()
    ("print-compile-error", "print out compile error")
    ("print-trace", "print trace")
    ("print-warning", "print warning")
    ("print-output-path", "print output path")
    ("print-compile-info", "print compile success or error")
    ("print-compile-info-dot", "print compile success or error by colorred dots")
    ("print-build-rate", "print build rate")
    ("print-test-info", "print test success or error information")
    ("print-test-info-dot", "print test info in colorred dots")
    ("print-csv", "print csv")
    ("print-csv-summary", "print csv summary")
    ("print-io-spec", "print IO spec") // DEPRECATED
    ("print-analysis-result", "print analysis result")
    ("print-main", "print main function")
    ("print-unresolved-id", "print unresolved ID in snippet registry") // DEPRECATED
    ;

  po::options_description debug_options("Debug Options");
  debug_options.add_options()
    ("pause-compile-error", "pause when compile error happens")
    ("pause-ast-unkonwn-tag", "pause when encoutner an unknown AST tag")
    ;
    
  /**
   * Enable a config option to be set via command line argument.
   * This will check and set the corresponding option in config file.
   * But, it will check whether the entry exists, and if not, output error message and exit.
   */
  po::options_description config_options("Config");
  config_options.add_options()
    ("cc", po::value<std::string>(), "c compiler used for compiling generated code")
    // CONFIRM if the bool here is just enter something
    ("run-test", po::value<bool>(), "whether to run test or not")
    // CONFIRM in the case of an int, can I get a string out of it?
    ("test-number", po::value<int>(), "Number of test")
    ("instrument-helium-guard", po::value<bool>(), "turn on helium guard")
    ("context-search-limit", po::value<int>(), "context search limit") // DEPRECATED
    ("code-selection", po::value<std::string>(), "code selection method") // DEPRECATED
    ("procedure-limit", po::value<int>(), "procedure limit for context search") // DEPRECATED
    ("context-search-only", po::value<bool>(), "Do context search only") // DEPRECATED
    ("context-search-step", po::value<bool>(), "context search step") // DEPRECATED
    ("test-global-variable", po::value<bool>(), "test global variable or not")
    ("address-sanitizer", po::value<bool>(), "use andress-sanitizer")
    ("gnulib", po::value<bool>(), "use gnulib in makefile")
    ("instrument-strlen", po::value<bool>(), "instrument strlen")
    ("instrument-address", po::value<bool>(), "instrument address")
    ("instrument-null", po::value<bool>(), "instrument null")
    ("max-strlen", po::value<int>(), "max strlen for test generation")
    ("int-min", po::value<int>(), "int min")
    ("int-max", po::value<int>(), "int max")
    ("max-array-size", po::value<int>(), "max-array-size")
    ("max-argv0-length", po::value<int>(), "max-argv0-length")
    ;
  
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("folder", "project folder")
    ;
  m_cmdline_options
    .add(general_options)
    .add(util_options)
    .add(config_options)
    .add(hidden)
    ;

  // Further group
  // all options avaliable for command line options
  // this is the message that will show for help messages
  m_help_options
    .add(general_options)
    .add(util_options)
    .add(config_options)
    ;

  m_config_options
    .add(config_options)
    .add(print_options)
    .add(debug_options)
    ;
  // this "folder" option include only one item
  m_positional.add("folder", 1);
}

void HeliumOptions::ParseCommandLine(int argc, char* argv[])
{
  /* run parser and store in m_vm */
  po::store(
            po::command_line_parser(argc, argv)
            .options(m_cmdline_options) // add cmdline options
            .positional(m_positional)     // add positional options
            .run(),                     // run the parser
            // store into m_vm
            m_vm
            );
  po::notify(m_vm);
}

void HeliumOptions::ParseConfigFile(std::string config_file) {
  std::ifstream ifs(config_file.c_str());
  po::store(po::parse_config_file(ifs, m_config_options), m_vm);
  po::notify(m_vm);
}

void HeliumOptions::PrintHelp() {
  std::cout<< "Usage: helium [options] <folder>" <<std::endl;
  std::cout<< m_help_options << std::endl;
  std::cout << "Refer to manpage of helium(1) for details."  << "\n";
}
