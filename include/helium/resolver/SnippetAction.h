#ifndef SNIPPETACTION_H
#define SNIPPETACTION_H

/**
 * Action for traverse code and create snippet
 */

#include "helium/resolver/SnippetV2.h"

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

/**
 * Create snippet for file.
 * This is the only API
 */
std::vector<v2::Snippet*> createSnippets(fs::path file);


#endif /* SNIPPETACTION_H */
