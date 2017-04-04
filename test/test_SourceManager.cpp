#include "helium/parser/source_manager.h"
#include "helium/utils/fs_utils.h"

#include <gtest/gtest.h>



#include <gtest/gtest.h>
#include <vector>
#include <string>


#include <fstream>


#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;
using namespace v2;

using namespace std;


TEST(SourceManagerTest, MyTest) {
  fs::path user_home = getenv("HOME");
  fs::path bench = user_home / "github" / "benchmark" / "craft" / "function";
  fs::path temp_dir = fs::temp_directory_path();
  {
    SourceManager *sourceManager = new SourceManager(bench);

    std::set<v2::ASTNodeBase*> sel = sourceManager->genRandSel(1);

    std::ofstream os;
    fs::path unique_p = fs::unique_path(temp_dir / "%%%%-%%%%-%%%%-%%%%");
    os.open(unique_p.string().c_str());
    ASSERT_TRUE(os.is_open());
    sourceManager->dumpSelection(sel, os);
    os.close();

    // std::cout << "Wrote to " << unique_p << "\n";

    // std::cout << "Content: " << "\n";
    // std::cout << utils::read_file(unique_p.string()) << "\n";

    std::set<v2::ASTNodeBase*> sel_load = sourceManager->loadSelection(unique_p);
    
    EXPECT_EQ(sel, sel_load);

    // std::cout << "sel: ";
    // for (auto *node : sel) {
    //   node->dump(std::cout);
    // }
    // std::cout << "\n";
    // std::cout << "sel_load: ";
    // for (auto *node : sel_load) {
    //   node->dump(std::cout);
    // }
    // std::cout << "\n";
  }
}
