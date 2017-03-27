#ifndef CLANG_SNIPPET_H
#define CLANG_SNIPPET_H
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Tooling/CommonOptionsParser.h"

#include <sqlite3.h>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
namespace fs = boost::filesystem;

struct ClangSnippetData {
  std::string name;
  std::string type;
  std::string file;
  unsigned begin_line;
  unsigned begin_column;
  unsigned end_line;
  unsigned end_column;
  ClangSnippetData(std::string name, std::string type, std::string file,
                   unsigned begin_line, unsigned begin_column, unsigned end_line, unsigned end_column)
    : name(name), type(type), file(file), begin_line(begin_line), begin_column(begin_column),
      end_line(end_line), end_column(end_column) {}
};


std::vector<ClangSnippetData> clangSnippetGetData();
void clangSnippetClearData();

class MyAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile);
  std::vector<ClangSnippetData> getData();
};


/**
 * Run front end action and get data
 */
void clangSnippetRun(fs::path target_cache_dir);
/**
 * Create database file and table
 */
void clangSnippetCreateDb(fs::path target_cache_dir);
void clangSnippetLoadDb(fs::path target_cache_dir);

/**
 * Insert data
 */
void clangSnippetInsertDb();


// query

std::string clangSnippetGetCode(std::string file, std::string kind, int line);

std::set<std::string> clangSnippetGetCallee(std::string caller);


#endif /* CLANG_SNIPPET_H */
