#include "arg_parser.h"
#include "utils/utils.h"
#include <iostream>
#include "options.h"

#include <gtest/gtest.h>

ArgParser *ArgParser::m_instance = NULL;

/*******************************
 ** ArgParser
 *******************************/

void ArgParser::Set(int argc, char* argv[])
{
  // create options_description
  po::options_description options("Arguments");
  // add options to it
  options.add_options()
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
    ("conf", po::value<std::string>(), "key=value pair of configure to owerwrite items in helium.conf")
    /**
     * Using implicit_value(), you tell po that, it can accept 0 or 1 token.
     * But if you use default_value(), po will throw exception if you provide no token.
     */
    ("print,p", po::value<std::string>()->implicit_value(""), "what to be print")
    ("debug,d", po::value<std::string>()->implicit_value(""), "debugging pause point")
    ("slice", po::value<std::string>(), "slice to use, as the code selection. THIS WILL SET THE CODE SELECTION METHOD TO SLICE")
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
    

  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("folder", "project folder")
    // config options. Should be kept the same as in helium.conf file
    // ("analyze-timeout", po::value<std::string>())
    // ("analyze-type", po::value<std::string>())
    // ("code-selection", po::value<std::string>())
    // ("context-search-method", po::value<std::string>())
    // ("context-search-value", po::value<std::string>())
    // ("instrument-position", po::value<std::string>())
    // ("instrument-type", po::value<std::string>())
    // ("max-context-size", po::value<std::string>())
    // ("max-segment-size", po::value<std::string>())
    // ("max-snippet-number", po::value<std::string>())
    // ("max-snippet-size", po::value<std::string>())
    // ("output-folder", po::value<std::string>())
    // ("segment-timeout", po::value<std::string>())
    // ("test-generation-method", po::value<std::string>())
    // ("run-test", po::value<std::string>())
    // ("test-number", po::value<std::string>())
    // ("test-timeout", po::value<std::string>())
    ;
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
  PrintOption::Instance()->Load(GetString("print"));
  DebugOption::Instance()->Load(GetString("debug"));
}

void ArgParser::PrintHelp() {
  if (Has("print") && GetString("print").empty()) {
    // std::cout << "--print -p help"  << "\n";
    PrintOption::Instance()->Help();
  } else if (Has("debug") && GetString("debug").empty()) {
    // std::cout << "--debug -d help"  << "\n";
    DebugOption::Instance()->Help();
  } else {
    std::cout<< "Usage: helium [options] <folder>" <<std::endl;
    std::cout<< m_help_options << std::endl;
    std::cout << "Refer to manpage of helium(1) for details."  << "\n";
    // std::cout << ""  << "\n";
    // std::cout << "Important options:"  << "\n";
    // std::cout << "\t" << "-c src"  << "\n";
    // std::cout << "\t" << "--conf='cc=clang'"  << "\n";
    // std::cout << "\t" << "--poi=../../ncompress-poi.txt"  << "\n";
    // std::cout << "Important instrument options: (--conf)"  << "\n";
    // std::cout << "\t" << "test-number=100"  << "\n";
    // std::cout << "\t" << "test-global-variable=true"  << "\n";
    // std::cout << "\t" << "instrument-address=true"  << "\n";
    // std::cout << "\t" << "instrument-strlen=true"  << "\n";
    // std::cout << "\t" << "instrument-null=true"  << "\n";
    // std::cout << "Important print options: (--print)"  << "\n";
    // std::cout << "\t" << "--print='csvsum,ana'"  << "\n";
    
    // std::cout << ""  << "\n";
    // std::cout << "Examples:"  << "\n";
    // std::cout << "\t" << "helium -s snippets/ cpped/ --print='ci,ce,col' --conf='instrument-strlen=true' -c src --whole-poi=/tmp/poi/poi.org -b gzip-1.2.4"  << "\n";
    // std::cout << "Buffer Overflow bugs:"  << "\n";
    // std::cout << "\t" << "helium -s snippets/ cpped/ --print='ci,ce,col' --conf='instrument-strlen=true'"  << "\n";
    // std::cout << "\t" << "helium -s snippets/ cpped/ --print='ci,ce,col' --conf='instrument-strlen=true,test-number=30' -c src --poi=../gzip-poi.txt"  << "\n";
    // std::cout << "\t" << "helium -s snippets/ cpped/ --print='ci,ce,col' --conf='instrument-strlen=true, test-number=100'"  << "\n";
    // std::cout << "Double free bugs:"  << "\n";
    // std::cout << "\t" << "helium -s snippets/ cpped/ --print='ci,ce,col' --conf='instrument-address=true'"  << "\n";
    // std::cout << "Null Dereference bugs:"  << "\n";
    // std::cout << "\t" << "helium -s snippets/ cpped/ --print='ci,ce,col' --conf='instrument-address=true, instrument-null=true'"  << "\n";
    // std::cout << ""  << "\n";
    // std::cout << "Preprocessing:"  << "\n";
    // std::cout << "\t" << "CC=gcc-6 helium-create-snippetdb.sh /path/to/folder"  << "\n";
    // std::cout << "\t" << "ps -ef | grep 'helium -s' | awk '{print $2}' | xargs kill"  << "\n";
    // std::cout << "\t" << "helium-run-parrel . 100"  << "\n";
  }
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

/**
 * commented out to suppress warning.
 */
// TEST(arg_parser_test_case, DISABLED_arg_parser) {
//   int argc = 3;
//   char *argv[] =
//     {"helium", "--context-search-method=hello-world", "folder"};
//   ArgParser args(argc, argv);
//   EXPECT_FALSE(args.Has("context-search-value"));
//   EXPECT_TRUE(args.Has("context-search-method"));
//   EXPECT_EQ(args.GetString("context-search-method"), "hello-world");
// }
