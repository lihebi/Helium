#ifndef OPTIONS_H
#define OPTIONS_H

#include "common.h"

void print_trace(const std::string &s);

typedef enum {
  POK_CompileError // print compile error
  , POK_AddSnippet // print add snippet detail
  , POK_AddSnippetDot // only print dot when adding snippet
  , POK_Segment // print current segment code
  , POK_Trace // print traces
  , POK_UnresolvedID // print unresolved id
  , POK_SegNo // current segment number
  , POK_CodeOutputLocation // output folder location
  , POK_CompileInfo // compile info
  , POK_CompileInfoDot // red dot for compile error, green dot for compile success
  , POK_BuildRate
  
  , POK_ExpAST // special for AST build rate experiment
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
  DOK_PauseCompileError,
  DOK_PauseASTUnknownTag // ast unknown tag: "should not reach here if I have a complete list."
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



#endif /* OPTIONS_H */
