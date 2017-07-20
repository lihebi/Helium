#include <gtest/gtest.h>

#include "helium/utils/FSUtils.h"

#include "helium/type/Cache.h"
#include "helium/type/SnippetAction.h"
#include "helium/type/Snippet.h"

#include "helium/parser/SourceManager.h"


#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;

using namespace std;

#if 0
TEST(GeneratorTest, MyTest) {
  fs::path user_home = getenv("HOME");
  fs::path bench = user_home / "github" / "benchmark" / "craft" / "function";
  {
    SourceManager *sourceManager = new SourceManager(bench);
    fs::path sel_file = bench / "single.sel";
    std::set<ASTNodeBase*> sel = sourceManager->loadSelection(sel_file);
    sel = sourceManager->loadSelection(sel_file);
    sel = sourceManager->grammarPatch(sel);
    sel = sourceManager->defUse(sel);
    sel = sourceManager->grammarPatch(sel);
    // std::cout << sel.size() << "\n";
    std::string prog = sourceManager->generateProgram(sel);
    // PRINT this out to test
    // std::cout << prog << "\n";
  }
  {
    SourceManager *sourceManager = new SourceManager(bench);
    fs::path sel_file = bench / "double.sel";
    std::set<ASTNodeBase*> sel = sourceManager->loadSelection(sel_file);
    sel = sourceManager->loadSelection(sel_file);
    sel = sourceManager->grammarPatch(sel);
    sel = sourceManager->defUse(sel);
    sel = sourceManager->grammarPatch(sel);
    // std::cout << sel.size() << "\n";
    std::string prog = sourceManager->generateProgram(sel);
    // std::cout << prog << "\n";
  }
}

#endif
