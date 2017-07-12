#include "helium/utils/HeliumOptions.h"
#include "helium/utils/Utils.h"
#include "helium/utils/FSUtils.h"
#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

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
    ("output,o", po::value<std::string>(), "output location")
    ("preprocess", "use c preprocessor to do preprocessing")
    ("create-snippet", "analyze and snippet")
    ("create-selection", "create random selection")
    ("create-headerdep", "create header dependence")
    ("create-include-dep", "create include manager json file")
    ("run", "run helium")
    ("selection", po::value<std::string>(), "selection folder or file")
    ("snippet", po::value<std::string>(), "snippet json file")
    ("include-dep", po::value<std::string>(), "include manager file")
    ;

  po::options_description primary_options("Primary");
  primary_options.add_options()
    ("create-cache", "create cache")
    ("ls-cache", "show cached projects")
    ("rm-cache", "remove cache")
    ("create-sel", "create selection")
    ("sel-num", po::value<int>()->default_value(10), "how many to generate")
    ("sel-tok", po::value<int>()->default_value(1), "how many token to select")
    ("sel", po::value<std::string>(), "selection file")
    ("dump-ast", "dump ast")
    ("hebi", "Experimental")
    ("system-info", "show system info")
    // ("discover-header", "discover header used in the benchmark on current system.")
    // ("check-header", "check header in project but not exists on current system or conf")
    ("bench-info", "show the information about target benchmark")
    ("check-headers", "check whether the headers is captured/supported")
    ("check-headers-bench", "check the bench for unsupported headers")
    ("check-cache-valid", "check whether the bench is valid by the snippets in cache")
    ;

  po::options_description util_options("Utils");
  util_options.add_options()
    // ("create-tagfile", "create tag file")
    ("create-snippet-db", "create snippet database")
    ("create-header-dep", "create header dependence table")
    
    ("show-header-dep", "the new dep printing: from the database")
    ("show-callgraph", "print callgraph")
    ("show-config", "print current config")
    ("show-headers", "print header")
    ("show-meta", "print meta data")
    ("show-cfg", "print CFG for each function")
    ("show-ast", "print AST for each function")


    ("show-instrument-code", po::value<std::string>(), "print instrument code")

    // ("check-headers", "check if the headers in headers.conf exists on this machine")
    ("create-function-ast", "create ast for all the functions in the target benchmarks")
    ("resolve-system-type", "Resolve a system type and print out result")
    ;

  po::options_description print_options("Print Options");
  print_options.add_options()
    ("print-trace", po::value<bool>()->default_value(false), "print trace")
    ("print-warning", po::value<bool>()->default_value(false), "print warning")

    ("print-code-output-location", po::value<bool>()->default_value(false), "print output path")

    ("print-compile-error", po::value<bool>()->default_value(false), "print out compile error")
    ("print-compile-info", po::value<bool>()->default_value(false), "print compile success or error")
    ("print-compile-info-dot", po::value<bool>()->default_value(false), "print compile success or error by colorred dots")

    ("print-build-rate", po::value<bool>()->default_value(false), "print build rate")

    ("print-test-info", po::value<bool>()->default_value(false), "print test success or error information")
    ("print-test-info-dot", po::value<bool>()->default_value(false), "print test info in colorred dots")

    ("print-io-spec", po::value<bool>()->default_value(false), "print IO spec") // DEPRECATED
    ("print-csv", po::value<bool>()->default_value(false), "print csv")
    ("print-csv-summary", po::value<bool>()->default_value(false), "print csv summary")
    ("print-analysis-result", po::value<bool>()->default_value(false), "print analysis result")
    ("print-input-variables", po::value<bool>()->default_value(false), "print the specification of input variables")

    ("print-benchmark-name", po::value<bool>()->default_value(false), "print benchmark running")
    ("print-segment-meta", po::value<bool>()->default_value(false),
     "print meta data for segment, including size, #proc, #branch, #loop")
    ("print-test-meta", po::value<bool>()->default_value(false),
     "print the testing data, including input number, generation time, total time used, "
     "stmt-cov, branch-cov, pass number, failure number")
    ("print-segment-peek", po::value<bool>()->default_value(false), "Peek the head of the segment")
    ("segment-peek-loc", po::value<int>()->default_value(3), "How many lines to peek")

    ("print-main", po::value<bool>()->default_value(false), "print main function")
    ("print-unresolved-id", po::value<bool>()->default_value(false), "print unresolved ID in snippet registry") // DEPRECATED
    ("print-analyze-result-transfer", po::value<bool>()->default_value(false), "print the transfer functions")
    ("print-analyze-result-meta", po::value<bool>()->default_value(false), "print the meta")

    ("dump-compile-error", po::value<bool>()->default_value(false), "dump compile error message into a file")
    ("print-sat-stmt", po::value<bool>()->default_value(false), "print sat stmt")
    ("print-sat-output", po::value<bool>()->default_value(false), "print sat output")
    ;

  po::options_description debug_options("Debug Options");
  debug_options.add_options()
    ("pause-compile-error", po::value<bool>()->default_value(false), "pause when compile error happens")
    ("pause-ast-unkonwn-tag", po::value<bool>()->default_value(false), "pause when encoutner an unknown AST tag")
    ("pause-no-testcase", po::value<bool>()->default_value(false), "pause when there's no test case generated")
    ;
    
  /**
   * Enable a config option to be set via command line argument.
   * This will check and set the corresponding option in config file.
   * But, it will check whether the entry exists, and if not, output error message and exit.
   */
  po::options_description config_options("Config");
  config_options.add_options()
    ("cc", po::value<std::string>(), "c compiler used for compiling generated code")
    ("poi-file", po::value<std::string>(), "POI csv file")

    ("helium-home", po::value<std::string>(), "Helium home folder")


    ("address-sanitizer", po::value<bool>()->default_value(false), "use andress-sanitizer")
    ("gnulib", po::value<bool>()->default_value(false), "use gnulib in makefile")

    ("run-test", po::value<bool>()->default_value(false), "whether to run test or not")
    ("instrument-io", po::value<bool>()->default_value(true), "Whether to instrument IO")
    ("test-global-variable", po::value<bool>()->default_value(false), "test global variable or not")
    ("test-generation-method", po::value<std::string>(), "test generation method")
    ("random-test-number", po::value<int>(), "Number of test")
    ("pairwise-test-number", po::value<int>(), "randomly select up to this number of tests from the pool of pairwise")
    ("pairwise-corner-number", po::value<int>(), "up to how many corner cases are used in pairwise")
    ("pairwise-random-number", po::value<int>(), "how many random values added to pairwise pool")
    ("test-timeout", po::value<int>(), "timeout for a test")

    // ("aggressive-merge", po::value<bool>()->default_value(false), "aggressively merge everything")
    // ("random-merge", po::value<bool>()->default_value(false), "random merge")
    // ("no-merge", po::value<bool>()->default_value(false), "no merge")
    ("merge-method", po::value<std::string>(), "merge method")
    
    ("aggressive-remove", po::value<bool>()->default_value(false), "remove whenever the transfer does not change")
    ("debug-remove-alg", po::value<bool>()->default_value(false, "debug removing algorithm"))

    ("test-input-max-strlen", po::value<int>(), "max strlen for test generation")
    ("test-input-min-int", po::value<int>(), "int min")
    ("test-input-max-int", po::value<int>(), "int max")
    ("test-input-max-array-size", po::value<int>(), "max-array-size")
    ("test-input-max-argv0-strlen", po::value<int>(), "max-argv0-length")
    ("test-input-max-pointer-size", po::value<int>(), "max malloc size for a pointer")
    ("test-null-pointer-percent", po::value<int>(), "the percent of generating a NULL for a pointer")

    ("procedure-limit", po::value<int>(), "procedure limit for context search")
    ("segment-per-poi-limit", po::value<int>()->default_value(-1),
     "how many segment to try. Will return after this number of segment is processed. -1 to disable")
    ("valid-poi-limit", po::value<int>()->default_value(-1), "valid poi limit")
    ("compile-error-limit-per-poi", po::value<int>()->default_value(-1), "compiler error limit")
    ("remove-branch-if-not-covered", po::value<bool>()->default_value(false), "Remove branch as long as POI is not covered.")
    ("gcov-handle-sigsegv", po::value<bool>()->default_value(false), "Dump coverage information when segment fault happen.")

    ("use-struct-type", po::value<bool>()->default_value(false), "Use structure type for input or just use unknown")
    ("use-query-resolver-2", po::value<bool>()->default_value(false), "use query resolver 2")
    ("negate-fc", po::value<bool>()->default_value(false), "negate the fc")
    ("analyze-data-option", po::value<std::string>(), "data for analyze")
    ("header-config-json", po::value<std::vector<std::string> >(), "header config files in json")
    ("header-disabled-packages", po::value<std::vector<std::string> >(), "header disabled package")
    ("header-disabled-include-paths", po::value<std::vector<std::string> >(), "disable include path")
    ("header-valid-include-paths",  po::value<std::vector<std::string> >(), "valid include path")
    ;
  
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("target", "target benchmark folder or file")
    ;
  
  m_cmdline_options
    .add(general_options)
    .add(primary_options)
    .add(util_options)
    .add(config_options)
    .add(print_options)
    .add(debug_options)
    .add(hidden)
    ;

  // Further group
  // all options avaliable for command line options
  // this is the message that will show for help messages
  m_help_options
    .add(general_options)
    .add(primary_options)
    // .add(util_options)
    // .add(config_options)
    ;

  m_config_options
    .add(config_options)
    .add(print_options)
    .add(debug_options)
    ;
  // this "folder" option include only one item
  m_positional.add("target", 1);
}

void HeliumOptions::ParseCommandLine(int argc, char* argv[])
{

  if (argc==1) {
    PrintHelp();
    exit(0);
  }
  
  try {
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
  } catch (po::unknown_option e) {
    std::cerr << e.what() << "\n";
    std::cerr << "EE: Unknown option in command line." << "\n";
    exit(1);
  }
}

void HeliumOptions::ParseConfigFile(std::string config_file) {
  config_file = utils::escape_tide(config_file);
  std::ifstream ifs;
  ifs.open(config_file.c_str());
  if (!ifs.is_open()) {
    std::cerr << "EE: Cannot open config file " << config_file << "\n";
    exit(1);
  }
  try {
    po::store(po::parse_config_file(ifs, m_config_options), m_vm);
    po::notify(m_vm);
  } catch (po::unknown_option e) {
    std::cerr << e.what() << "\n";
    std::cerr << "EE: Unknown option in config file." << "\n";
  }
}

void HeliumOptions::PrintHelp() {
  std::cout<< "Usage: helium [options] <folder>" <<std::endl;
  std::cout<< m_help_options << std::endl;
  std::cout << "Refer to manpage of helium(1) for details."  << "\n";
}


std::string HeliumOptions::GetString(std::string key) {
  if (m_vm.count(key) == 1) {
    try {
      return m_vm[key].as<std::string>();
    } catch (boost::bad_any_cast) {
      std::cerr << "EE: Option " << key << " is not a string."  << "\n";
      exit(1);
    }
  } else {
    std::cerr << "EE: Option " << key << " is not set."  << "\n";
    exit(1);
  }
}
bool HeliumOptions::GetBool(std::string key) {
  if (m_vm.count(key) == 1) {
    try {
      return m_vm[key].as<bool>();
    } catch (boost::bad_any_cast) {
      std::cerr << "EE: Option " << key << " is not a bool."  << "\n";
      exit(1);
    }
  } else {
    std::cerr << "EE: Option " << key << " is not set."  << "\n";
    exit(1);
  }
}
int HeliumOptions::GetInt(std::string key) {
  if (m_vm.count(key) == 1) {
    try {
      return m_vm[key].as<int>();
    } catch (boost::bad_any_cast) {
      std::cerr << "EE: Option " << key << " is not a int."  << "\n";
      exit(1);
    }
  } else {
    std::cerr << "EE: Option " << key << " is not set."  << "\n";
    exit(1);
  }
}


std::vector<std::string> HeliumOptions::GetStringVector(std::string key) {
  if (m_vm.count(key) == 1) {
    try {
      return m_vm[key].as< std::vector<std::string> >();
    } catch (boost::bad_any_cast) {
      std::cerr << "EE: Option " << key << " is not a vector of string."  << "\n";
      exit(1);
    }
  } else {
    // std::cerr << "EE: Option " << key << " is not set."  << "\n";
    // exit(1);
    // returning empty
    return {};
  }
}


int makeargs(const char *args, int *argc, char ***aa) {
  char *buf = strdup(args);
  int c = 1;
  char *delim;
  char **argv = (char**)calloc(c, sizeof (char *));

  argv[0] = buf;

  while ((delim = strchr(argv[c - 1], ' '))) {
    argv = (char**)realloc(argv, (c + 1) * sizeof (char *));
    argv[c] = delim + 1;
    *delim = 0x00;
    c++;
  }

  *argc = c;
  *aa = argv;

  return c;
}

TEST(HeliumOptionsTestCase, ProgramOptionsTest) {
  po::options_description config_options("Config");
  config_options.add_options()
    ("string1", po::value<std::string>(), "c compiler used for compiling generated code")
    ("bool1", po::value<bool>(), "whether to run test or not")
    ("bool2", po::value<bool>(), "whether to run test or not")
    ("bool3", po::value<bool>(), "whether to run test or not")
    ("bool4", po::value<bool>(), "whether to run test or not")
    ("bool5", po::value<bool>()->default_value(false), "whether to run test or not")
    ("non1", "bool2 explained")
    ("non2", "bool2 explained")
    ("non3", "bool2 explained")
    ("int1", po::value<int>(), "Number of test")
    ("int2", po::value<int>(), "Number of test")
    ;

  int argc;
  char **argv;
  makeargs("helium"
           " --string1 sss"
           " --bool1 true --bool2 false --bool4 yes" // true, false, yes, no can be used!
           " --non1"
           " --int1 10 --int2 20"
           , &argc, &argv);
  
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv) .options(config_options) .run(), vm);
  po::notify(vm);
  
  EXPECT_EQ(vm["bool1"].as<bool>(), true);
  EXPECT_EQ(vm["bool2"].as<bool>(), false);
  EXPECT_EQ(vm.count("bool3"), 0);
  EXPECT_EQ(vm["bool4"].as<bool>(), true);
  EXPECT_EQ(vm["bool5"].as<bool>(), false); // default value will be in vm

  
  EXPECT_EQ(vm["int1"].as<int>(), 10);
  EXPECT_THROW(vm["int1"].as<std::string>(), boost::bad_any_cast); // int cannot be convert to string

  EXPECT_THROW(vm["non1"].as<bool>(), boost::bad_any_cast); // non cannot convert to bool
}
