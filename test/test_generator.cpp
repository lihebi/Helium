#include <gtest/gtest.h>

#include "helium/resolver/snippet.h"
#include "helium/resolver/snippet_db.h"
#include "helium/utils/fs_utils.h"

#include "helium/resolver/cache.h"
#include "helium/resolver/SnippetAction.h"
#include "helium/resolver/SnippetV2.h"

#include "helium/parser/source_manager.h"


#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;

using namespace std;

static void dummy() {
  SourceManager *sourceManager;
}

TEST(GeneratorTest, MyTest) {
  fs::path user_home = getenv("HOME");
  fs::path bench = user_home / "github" / "benchmark" / "craft" / "function";
  {
    SourceManager *sourceManager = new SourceManager(bench);
    fs::path sel_file = bench / "single.sel";
    std::set<v2::ASTNodeBase*> sel = sourceManager->loadSelection(sel_file);
    sel = sourceManager->loadSelection(sel_file);
    sel = sourceManager->grammarPatch(sel);
    sel = sourceManager->defUse(sel);
    sel = sourceManager->grammarPatch(sel);
    // std::cout << sel.size() << "\n";
    std::string prog = sourceManager->generateProgram(sel);
    std::cout << prog << "\n";
  }
  {
    SourceManager *sourceManager = new SourceManager(bench);
    fs::path sel_file = bench / "double.sel";
    std::set<v2::ASTNodeBase*> sel = sourceManager->loadSelection(sel_file);
    sel = sourceManager->loadSelection(sel_file);
    sel = sourceManager->grammarPatch(sel);
    sel = sourceManager->defUse(sel);
    sel = sourceManager->grammarPatch(sel);
    // std::cout << sel.size() << "\n";
    std::string prog = sourceManager->generateProgram(sel);
    std::cout << prog << "\n";
  }
}
