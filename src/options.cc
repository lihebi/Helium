#include "options.h"
#include "utils.h"

#include <iostream>
/*******************************
 ** Print Options
 *******************************/
PrintOption *PrintOption::m_instance = 0;

static const std::map<std::string, PrintOptionKind> POK_MAP {
  {"compile-error", POK_CompileError}
  , {"ce", POK_CompileError}
  , {"add-snippet", POK_AddSnippet}
  , {"as", POK_AddSnippet}
  , {"asd", POK_AddSnippetDot}
  , {"t", POK_Trace}
  , {"trace", POK_Trace}
  , {"un", POK_UnresolvedID}
  , {"seg", POK_Segment}
  , {"segno", POK_SegNo}
  , {"ctx", POK_Context}
  , {"main", POK_Main}
  , {"col", POK_CodeOutputLocation}
  , {"ci", POK_CompileInfo}
  , {"cid", POK_CompileInfoDot}
  , {"ti", POK_TestInfo}
  , {"tid", POK_TestInfoDot}
  , {"br", POK_BuildRate}
  , {"exp-ast", POK_ExpAST}
  , {"io", POK_IOSpec}
  , {"ana", POK_AnalysisResult}
  , {"csvsum", POK_CSVSummary}
};

/**
 * print help information for print option.
 */
void PrintOption::Help() {
  std::cout << "available print option (--print 'xx,yy'):\n";

  std::cout << "\t ce: compile-error"  << "\n";
  std::cout << "\t as: add-snippet"  << "\n";
  std::cout << "\t t: trace"  << "\n";
  std::cout << "\t un: unresolved ids when resolving snippets"  << "\n";
  std::cout << "\t seg: print out the segment."  << "\n";
  std::cout << "\t segno: print current segment NO."  << "\n";
  std::cout << "\t ctx: print current context"  << "\n";
  std::cout << "\t main: print the main.c file (not only the main function)"  << "\n";
  std::cout << "\t col: code output location"  << "\n";
  std::cout << "\t ci: compile information(success or fail)"  << "\n";
  std::cout << "\t cid: compile information as dot"  << "\n";
  std::cout << "\t ti: test information (success or not)"  << "\n";
  std::cout << "\t tid: test information as dot"  << "\n";
  std::cout << "\t br: build rate"  << "\n";
  std::cout << "\t exp-ast: AST build rate special"  << "\n";
  std::cout << "\t io: IO specifications during testing"  << "\n";
  std::cout << "\t ana: analysis result"  << "\n";
  std::cout << "\t csvsum: csv summary"  << "\n";
}


/**
 * s should be a string separated by comma.
 */
void PrintOption::Load(std::string s) {
  m_kinds.clear();
  std::vector<std::string> options = utils::split(s, ',');
  for (std::string option : options) {
    utils::trim(option);
    try {
      m_kinds.insert(POK_MAP.at(option));
    } catch (std::out_of_range &e) {
      std::cerr << "Print Option: " << option << " not recognized.\n";
      assert(false);
    }
  }
}

void print_trace(const std::string &s) {
  if (PrintOption::Instance()->Has(POK_Trace)) {
    std::cout << "[trace] " << s  << "\n";
  }
}
bool PrintOption::Has(PrintOptionKind kind) {
  return m_kinds.count(kind) == 1;
}

/*******************************
 ** Debug Options
 *******************************/
DebugOption *DebugOption::m_instance = 0;
static const std::map<std::string, DebugOptionKind> DOK_MAP {
  {"compile-error", DOK_PauseCompileError}
  , {"ce", DOK_PauseCompileError}
  , {"ast", DOK_PauseASTUnknownTag}
};

/**
 * s should be a string separated by comma.
 */
void DebugOption::Load(std::string s) {
  m_kinds.clear();
  std::vector<std::string> options = utils::split(s, ',');
  for (std::string option : options) {
    utils::trim(option);
    try {
      m_kinds.insert(DOK_MAP.at(option));
    } catch (std::out_of_range &e) {
      std::cerr << "Debug Option: " << option << " not recognized.\n";
      assert(false);
    }
  }
}
/**
 * debug help information for debug option.
 */
void DebugOption::Help() {
  std::cout << "available debug option (--debug 'xx,yy'):\n";

  std::cout << "\tce: compile-error: pause when compile error occurs."  << "\n";
  std::cout << "\tast: AST unknown tag"  << "\n";
}
bool DebugOption::Has(DebugOptionKind kind) {
  return m_kinds.count(kind) == 1;
}

