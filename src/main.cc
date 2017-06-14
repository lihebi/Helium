#include <iostream>
#include <cstdlib>

#include "helium/workflow/helium.h"

#include "helium/resolver/snippet_db.h"
#include "helium/parser/cfg.h"
#include "helium/parser/xml_doc_reader.h"
#include "helium/parser/ast_node.h"

#include "helium/parser/parser.h"

#include "helium/utils/helium_options.h"
#include "helium/parser/point_of_interest.h"
#include "helium/utils/fs_utils.h"
#include "helium/utils/utils.h"

#include "helium/parser/source_manager.h"

#include "helium/resolver/clangSnippet.h"
#include "helium/resolver/cache.h"

#include "helium/resolver/SnippetV2.h"
#include "helium/resolver/SnippetAction.h"


#include <gtest/gtest.h>
#include <sqlite3.h>


#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;

using std::vector;
using std::string;
using std::set;
using std::map;
using std::pair;


/**
 * Do compilation in dir
 */
bool do_compile(fs::path dir) {
  std::string clean_cmd = "make clean -C " + dir.string();
  std::string cmd = "make -C " + dir.string();
  cmd += " 2>&1";
  utils::exec(clean_cmd.c_str(), NULL);
  int return_code;
  std::string error_msg = utils::exec_sh(cmd.c_str(), &return_code);
  std::cout << "[main] Error Message:" << "\n";
  std::cout << error_msg << "\n";
  if (return_code == 0) {
    return true;
  } else {
    return false;
  }
}

std::vector<fs::path> load_sel(fs::path sel_dir) {
  // get selection files
  std::vector<fs::path> ret;
  if (HeliumOptions::Instance()->Has("sel")) {
    fs::path sel_config = HeliumOptions::Instance()->GetString("sel");
    if (fs::is_regular_file(sel_config)) {
      ret.push_back(sel_config);
    }
    if (fs::is_directory(sel_config)) {
      fs::recursive_directory_iterator it(sel_dir), eod;
      BOOST_FOREACH (fs::path const & p, std::make_pair(it, eod)) {
        // must be .sel file
        if (is_regular_file(p) && p.extension() == ".json") {
          ret.push_back(sel_config);
        }
      }
    }
  } else {
    if (fs::is_directory(sel_dir)) {
      fs::recursive_directory_iterator it(sel_dir), eod;
      BOOST_FOREACH (fs::path const & p, std::make_pair(it, eod)) {
        // must be .sel file
        if (is_regular_file(p) && p.extension() == ".json") {
          ret.push_back(p);
        }
      }
    }
  }
  return ret;
}


void helium_run(fs::path target, fs::path target_cache_dir) {


  // to count how many sels. this variable will be stored in
  // helium_var.txt and will not be cleared unless remove the file
  int sel_count=0;
  fs::path user_home(getenv("HOME"));
  fs::path helium_home = user_home / ".helium.d";
  // load "~/helium.d/helium_var.txt"
  if (fs::exists(helium_home / "helium_var.txt")) {
    std::ifstream is((helium_home / "helium_var.txt").string());
    is >> sel_count;
    is.close();
  }
  
  // (HEBI: Running)
  // Put the selection file in specific folder, Helium will find them itself.
  // Which folder to select
  // std::vector<fs::path> sels = load_sel(target_cache_dir / "sel");
  std::vector<fs::path> sels = load_sel(target_cache_dir / "iclonesel");
  std::cout << "[main] Running Helium on " << target.string() << " .." << "\n";
  // fs::path target_sel_dir = helium_home / "sel" / target_cache_dir_name;
  
  fs::path gen_program_dir = target_cache_dir / "gen";
  if (!fs::exists(gen_program_dir)) fs::create_directories(gen_program_dir);
  SourceManager *sourceManager = new SourceManager(target_cache_dir / "cpp");
  for (fs::path sel_file : sels) {
    sel_count++;

    std::cerr << "[PROCESS] Global Sel Count: " << sel_count << "\n";
    std::ofstream os((helium_home / "helium_var.txt").string());
    os << sel_count;
    os.close();

    
    // std::set<v2::ASTNodeBase*> sel = sourceManager->loadSelection(sel_file);
    std::set<v2::ASTNodeBase*> sel = sourceManager->loadJsonSelection(sel_file);
    std::cout << "---------------------" << "\n";
    std::cout << "[main] Rerun this by: helium " << target.string() << " --sel " << sel_file.string() << "\n";
    std::cout << "[main] Selected " << sel.size() << " tokens on Selection file " << sel_file.string() << "\n";
    std::cout << "[main] Doing Grammar Patching .." << "\n";

    std::cout << "[main] Dump Dist for Sel: " << "\n";
    sourceManager->dumpDist(sel, std::cout);
    {
      std::set<v2::ASTNodeBase*> tmp = sourceManager->filterLeaf(sel);
      std::cout << "[main] Dump Dist for Sel Token: " << "\n";
      sourceManager->dumpDist(tmp, std::cout);
    }

    // (HEBI: Grammar Patch)
    sel = sourceManager->grammarPatch(sel);

    std::cout << "[main] Dump Dist for Patch: " << "\n";
    sourceManager->dumpDist(sel, std::cout);
    {
      std::set<v2::ASTNodeBase*> tmp = sourceManager->filterLeaf(sel);
      std::cout << "[main] Dump Dist for Patch Token: " << "\n";
      sourceManager->dumpDist(tmp, std::cout);
    }
    
    
    // (HEBI: Def-use)
    sel = sourceManager->defUse(sel);
    // (HEBI: Grammar Patch)
    // I might want to do another grammar patching in case def use breaks it
    sel = sourceManager->grammarPatch(sel);
    std::cout << "[main] Patch size: "<< sel.size() << " ==> ";
    for (v2::ASTNodeBase *node : sel) {node->dump(std::cout);}
    std::cout << "\n";
      
    std::string prog = sourceManager->generateProgram(sel);
    std::cout << "[main] Program:" << "\n";
    std::cout << prog << "\n";

    // put the program into main function
    // TODO for function parameter, we need not include function header, but create variable??

    // Generate into folder and do compilation
    // use the sel filename as the folder
    fs::path gen_dir = gen_program_dir / sel_file.filename();
    sourceManager->generate(sel, gen_dir);
    std::cout << "[main] Code written to " << gen_dir.string() << "\n";
    // do compilation
    if (do_compile(gen_dir)) {
      std::cout << utils::GREEN << "[main] Compile Success !!!" << utils::RESET << "\n";
      std::cerr << "[main] Compile Success !!!" << "\n";

      // run tests
      std::cout << "[main] Running program in that target folder .." << "\n";
      std::string run_cmd = "make run -C " + gen_dir.string();
      // run_cmd += " 2>&1";
      utils::exec(run_cmd.c_str(), NULL);
      int return_code=-1;
      std::string error_msg = utils::exec_sh(run_cmd.c_str(), &return_code);
      std::cout << "[main] Output written to " << gen_dir.string() << "/helium_output.txt" << "\n";
      if (return_code == 0) {
        std::cout << "[main] Run Success" << "\n";
      } else {
        // This error message will be make error message
        // if the return code of a.out is not 0, it will be
        //   make: *** [Makefile:10: run] Error 1
        // if seg fault, it will be
        //   make: *** [Makefile:10: run] Segmentation fault (core dumped)
        std::cout << "[main] Run Failure" << "\n";
      }
    } else {
      std::cout << utils::RED << "[main] Compile Failure ..." << utils::RESET << "\n";
      std::cerr << "[main] Compile Failure ..." << "\n";
    }
  }
}

void general_utility() {
  fs::path user_home(getenv("HOME"));
  fs::path helium_home = user_home / ".helium.d";
  fs::path cache_dir = helium_home / "cache";
  if (HeliumOptions::Instance()->Has("ls-cache")) {
    for (fs::directory_entry &e : fs::directory_iterator(cache_dir)) {
      fs::path p = e.path();
      if (fs::is_directory(p)) {
        std::cout << p.filename().string() << "\n";}}
    exit(0);}
  if (HeliumOptions::Instance()->Has("rm-cache")) {
    std::string toremove = HeliumOptions::Instance()->GetString("target");
    if (fs::exists(cache_dir / toremove)) {
      fs::remove_all(cache_dir / toremove);
      exit(0);
    } else {
      std::cerr << "No such cache: " << toremove << "\n";
      exit(1);}}
  if (HeliumOptions::Instance()->Has("system-info")) {
    HeaderManager::Instance()->dump(std::cout);
    exit(0);
  }
}


void target_utility(fs::path target) {
  fs::path user_home(getenv("HOME"));
  fs::path helium_home = user_home / ".helium.d";
  if (HeliumOptions::Instance()->Has("check-headers")) {
    // checking if header is captured
    // read target
    // target should be a list of headers, one per line
    std::ifstream ifs(target.string());
    assert(ifs.is_open());
    std::string line;
    std::vector<std::string> suc;
    std::vector<std::string> fail;
    while (getline(ifs, line)) {
      utils::trim(line);
      if (!line.empty()) {
        bool res = HeaderManager::Instance()->jsonCheckHeader(line);
        if (res) suc.push_back(line);
        else fail.push_back(line);
      }
    }
    ifs.close();
    // output
    std::cout << "Found: ";
    for (std::string s : suc) {
      std::cout << s << " ";
    }
    std::cout << "\n";
    std::cout << "Not Found: ";
    for (std::string s : fail) {
      std::cout << s << " ";
    }
    std::cout << "\n";
    // exit(0);

    if (fail.size() == 0) exit(0);
    else exit(1);
  }
  if (HeliumOptions::Instance()->Has("check-headers-bench")) {
    // parse the bench
    HeaderManager::Instance()->jsonParseBench(target);
    // if (HeaderManager::Instance()->jsonValidBench()) {
    // }

    std::string reason;
    bool valid = HeaderManager::Instance()->jsonValidBench(reason);
    if (valid) exit(0);
    else {
      std::cout << reason << "\n";
      exit(1);
    }
  }
}

void helium_utility(fs::path target) {
  if (HeliumOptions::Instance()->Has("bench-info")) {
    std::cout << "== Header Dependencies:" << "\n";
    HeaderManager::Instance()->dumpDeps(std::cout);
    HeaderManager::Instance()->dump(std::cout);
    exit(0);
  }
  /**
   * check if the cache is valid
   * Need to create cache first
   * - check whether a function is defined multiple times
   */
  if (HeliumOptions::Instance()->Has("check-cache-valid")) {
    std::string reason;
    bool valid = v2::GlobalSnippetManager::Instance()->checkValid(reason);
    if (valid) exit(0);
    else {
      std::cout << "Invalid because " << reason << "\n";
      exit(1);
    }
  }
}

void check_blocklist(fs::path blockconf, std::string name) {
  std::ifstream ifs(blockconf.string());
  assert(ifs.is_open());
  std::string line;
  std::set<std::string> blocklist;
  while (std::getline(ifs, line)) {
    utils::trim(line);
    if (line.empty() || line[0]=='#') continue;
    std::string proj = utils::split(line)[0];
    blocklist.insert(proj);
  }
  ifs.close();
  for (std::string s : blocklist) {
    // std::cout << "checking " << s << "\n";
    if (name.find(s) != std::string::npos) {
      std::cout << "[main] This project is in blacklist. Skipped." << "\n";
      exit(0);
    }
  }
}

void create_cache(fs::path target, fs::path target_cache_dir) {
  fs::path target_sel_dir = target_cache_dir / "sel";
  // remove folder
  if (fs::exists(target_cache_dir)) fs::remove_all(target_cache_dir);

  // create everything
  fs::create_directories(target_cache_dir);
  std::cout << "== Creating src .." << "\n";
  create_src(target, target_cache_dir, target_sel_dir);
  std::cout << "== Creating cpp .." << "\n";
  create_cpp(target_cache_dir);

  // The new snippet system
  std::cout << "== Creating snippets, deps, outers ..." << "\n";
  v2::SnippetManager *snippet_manager = new v2::SnippetManager();
  snippet_manager->traverseDir(target_cache_dir / "cpp");

  // snippet_manager->dumpLight(std::cout);
  snippet_manager->dump(std::cout);

  // snippet_manager->save(target_cache_dir);
  snippet_manager->saveJson(target_cache_dir / "snippets.json");
}

void create_sel(fs::path target_cache_dir) {
  fs::path target_sel_dir = target_cache_dir / "sel";
  int sel_num = HeliumOptions::Instance()->GetInt("sel-num");
  int sel_tok = HeliumOptions::Instance()->GetInt("sel-tok");
  // create selection
  // create selection folder if not exist
  if (fs::exists(target_sel_dir)) fs::remove_all(target_sel_dir);
  fs::create_directories(target_sel_dir);
  SourceManager *sourceManager = new SourceManager(target_cache_dir / "cpp");
  // create and overwrite random/ folder
  fs::remove_all(target_sel_dir / "random");
  fs::create_directories(target_sel_dir / "random");

  for (int i=0;i<sel_num;i++) {
    std::set<v2::ASTNodeBase*> selection;
    // fs::path file = target_sel_dir / "random" / (std::to_string(i) + ".sel");
    fs::path file = target_sel_dir / "random" / (std::to_string(i) + ".json");
    std::ofstream os;
    os.open(file.string().c_str());
    assert(os.is_open());
    // selection = sourceManager->genRandSelSameFunc(1);
    // selection = sourceManager->genRandSel(sel_tok);
    selection = sourceManager->genRandSelFunc(sel_tok);
    // sourceManager->dumpSelection(selection, os);
    sourceManager->dumpJsonSelection(selection, os);
    os.close();
    std::cout << "Selection file wrote to " << file.string() << "\n";
  }
}

void load_header_config() {
  {
    std::vector<std::string> v;
    v = HeliumOptions::Instance()->GetStringVector("header-valid-include-paths");
    for (std::string s : v) {
      HeaderManager::Instance()->jsonAddValidIncludePath(s);
    }
  }
  {
    std::vector<std::string> v = HeliumOptions::Instance()->GetStringVector("header-config-json");
    for (std::string vi : v) {
      HeaderManager::Instance()->jsonAddConf(vi);
    }
  }
}


/**
 * \mainpage Helium Reference Manual
 *
 * You might want to browse the important classes. The modules page shows some grouped classes.
 */
int main(int argc, char* argv[]) {
  utils::seed_rand();
  const char *home = getenv("HOME");
  if (!home) {
    std::cerr << "EE: HOME env is not set." << "\n";
    exit(1);
  }

  /* parse arguments */
  HeliumOptions::Instance()->ParseCommandLine(argc, argv);
  HeliumOptions::Instance()->ParseConfigFile("~/.heliumrc");
  if (HeliumOptions::Instance()->Has("help")) {
    HeliumOptions::Instance()->PrintHelp();
    exit(0);}

  fs::path user_home(getenv("HOME"));
  fs::path helium_home = user_home / ".helium.d";
  if (!fs::exists(helium_home)) {
    fs::create_directory(helium_home);}
  fs::path cache_dir = helium_home / "cache";

  load_header_config();
  // (HEBI: Utility 1)
  general_utility();
  
  // Reading target folder. This is the parameter
  std::string target_str = HeliumOptions::Instance()->GetString("target");
  target_str = utils::escape_tide(target_str);
  fs::path target(target_str);

  // (HEBI: Utility 2)
  target_utility(target);

  if (!fs::exists(target)) {
    std::cerr << "EE: target folder or file " << target.string() << " does not exist." << "\n";
    exit(1);}
  // target is absolute path
  target = fs::canonical(target);
  std::string target_dir_name = target.string();
  std::replace(target_dir_name.begin(), target_dir_name.end(), '/', '_');
  fs::path target_cache_dir(helium_home / "cache" / target_dir_name);
  // fs::path target_sel_dir(helium_home / "sel" / target_dir_name);
  fs::path target_sel_dir = target_cache_dir / "sel";

  check_blocklist(helium_home / "etc" / "blacklist.conf", target_dir_name);


  if (HeliumOptions::Instance()->Has("create-cache")) {create_cache(target, target_cache_dir); exit(0);}
  if (HeliumOptions::Instance()->Has("create-sel")) {create_sel(target_cache_dir); exit(0);}

  if (!fs::exists(target_cache_dir)) {
    std::cerr << "The benchmark is not processed."
              << "Run helium --create-cache /path/to/benchmark" << "\n";
    exit(1);}

  std::cout << "[main] Loading header manager .." << "\n";
  HeaderManager::Instance()->jsonParseBench(target);
  HeaderManager::Instance()->adjustDeps(target.string(), (target_cache_dir / "cpp").string());
  HeaderManager::Instance()->jsonTopoSortHeaders();
  HeaderManager::Instance()->jsonResolve();
  fs::path snippet_file = target_cache_dir / "snippets.json";
  std::cout << "[main] Loading snippet from " << snippet_file << "\n";
  v2::GlobalSnippetManager::Instance()->loadJson(snippet_file);
  v2::GlobalSnippetManager::Instance()->processAfterLoad();
  v2::GlobalSnippetManager::Instance()->dump(std::cout);

  // (HEBI: Utility 3)
  helium_utility(target);

  helium_run(target, target_cache_dir);
  std::cout << "[main] End Of Helium" << "\n";
  return 0;
}
