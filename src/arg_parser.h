#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__


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

void print_trace(const std::string &s);

typedef enum {
  POK_CompileError,
  POK_AddSnippet,
  POK_Trace
} PrintOptionKind;
class PrintOption {
public:
  static PrintOption* Instance() {
    if (m_instance == NULL) {
      m_instance = new PrintOption();
    }
    return m_instance;
  }
  ~PrintOption();
  void Load(std::string);
  bool Has(PrintOptionKind kind);
  void Help();
private:
  PrintOption() {}
  static PrintOption *m_instance;
  std::set<PrintOptionKind> m_kinds;
};

/**
 * Debug options
 */

typedef enum {
  DOK_PauseCompileError
} DebugOptionKind;
class DebugOption {
public:
  static DebugOption* Instance() {
    if (m_instance == NULL) {
      m_instance = new DebugOption();
    }
    return m_instance;
  }
  ~DebugOption() {}
  void Load(std::string);
  bool Has(DebugOptionKind kind);
  void Help();
private:
  DebugOption() {}
  static DebugOption *m_instance;
  std::set<DebugOptionKind> m_kinds;
};


#endif
