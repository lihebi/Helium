#include <iostream>
#include <cstdlib>

#include "helium/utils/XMLDocReader.h"

#include "helium/parser/Parser.h"

#include "helium/utils/HeliumOptions.h"
#include "helium/utils/FSUtils.h"
#include "helium/utils/Utils.h"

#include "helium/parser/SourceManager.h"

#include "helium/type/ClangSnippet.h"
#include "helium/type/Cache.h"

#include "helium/type/Snippet.h"
#include "helium/type/SnippetAction.h"

#include "helium/utils/ThreadUtils.h"
#include "helium/utils/ColorUtils.h"
#include "helium/utils/StringUtils.h"
#include "helium/utils/RandUtils.h"


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
  ThreadExecutor(clean_cmd).run();
  std::string cmd = "make -C " + dir.string();
  cmd += " 2>&1";
  ThreadExecutor exe(cmd);
  exe.run();
  std::cout << "[main] Error Message:" << "\n" << exe.getStdOut() << "\n";
  if (exe.getReturnCode() == 0) {
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

std::vector<fs::path> get_selections(fs::path selection) {
  std::vector<fs::path> ret;
  if (fs::exists(selection)) {
    if (fs::is_regular_file(selection)) {
      // single selection file
      ret.push_back(selection);
    } else if (fs::is_directory(selection)){
      // multiple ret
      fs::recursive_directory_iterator it(selection), eod;
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
void run_on_selection(fs::path indir, fs::path outdir, fs::path selection,
                      SnippetManager *snip_manager,
                      IncludeManager *inc_manager,
                      LibraryManager *lib_manager) {
  SourceManager *sourceManager = new SourceManager(indir);
  std::vector<fs::path> sels = get_selections(selection);
  for (fs::path sel_file : sels) {
    std::set<ASTNodeBase*> sel = sourceManager->loadJsonSelection(sel_file);
    sourceManager->dumpDist(sel, std::cout);
    sel = sourceManager->grammarPatch(sel);
    sel = sourceManager->defUse(sel);
    // I might want to do another grammar patching in case def use breaks it
    sel = sourceManager->grammarPatch(sel);
    std::string prog = sourceManager->generateProgram(sel);
    
    // Generate into folder and do compilation
    // use the sel filename as the folder
    fs::path gen_dir = outdir / sel_file.filename();
    sourceManager->generate(sel, gen_dir);
    std::cout << "[main] Code written to " << gen_dir.string() << "\n";
    // do compilation
    if (do_compile(gen_dir)) {
      std::cerr << utils::GREEN << "[main] Compile Success !!!" << utils::RESET << "\n";

      // run tests
      std::cout << "[main] Running program in that target folder .." << "\n";
      std::string run_cmd = "make run -C " + gen_dir.string();
      // run_cmd += " 2>&1";
      ThreadExecutor exe(run_cmd);
      exe.run();
      std::cout << "[main] Output written to "
                << gen_dir.string() << "/helium_output.txt" << "\n";
      if (exe.getReturnCode() == 0) {
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

    
    // std::set<ASTNodeBase*> sel = sourceManager->loadSelection(sel_file);
    std::set<ASTNodeBase*> sel = sourceManager->loadJsonSelection(sel_file);
    std::cout << "---------------------" << "\n";
    std::cout << "[main] Rerun this by: helium " << target.string() << " --sel " << sel_file.string() << "\n";
    std::cout << "[main] Selected " << sel.size() << " tokens on Selection file " << sel_file.string() << "\n";
    std::cout << "[main] Doing Grammar Patching .." << "\n";

    std::cout << "[main] Dump Dist for Sel: " << "\n";
    sourceManager->dumpDist(sel, std::cout);
    {
      std::set<ASTNodeBase*> tmp = sourceManager->filterLeaf(sel);
      std::cout << "[main] Dump Dist for Sel Token: " << "\n";
      sourceManager->dumpDist(tmp, std::cout);
    }

    // (HEBI: Grammar Patch)
    sel = sourceManager->grammarPatch(sel);

    std::cout << "[main] Dump Dist for Patch: " << "\n";
    sourceManager->dumpDist(sel, std::cout);
    {
      std::set<ASTNodeBase*> tmp = sourceManager->filterLeaf(sel);
      std::cout << "[main] Dump Dist for Patch Token: " << "\n";
      sourceManager->dumpDist(tmp, std::cout);
    }
    
    
    // (HEBI: Def-use)
    sel = sourceManager->defUse(sel);
    // (HEBI: Grammar Patch)
    // I might want to do another grammar patching in case def use breaks it
    sel = sourceManager->grammarPatch(sel);
    std::cout << "[main] Patch size: "<< sel.size() << " ==> ";
    for (ASTNodeBase *node : sel) {node->dump(std::cout);}
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
      ThreadExecutor exe(run_cmd);
      exe.run();
      std::cout << "[main] Output written to "
                << gen_dir.string() << "/helium_output.txt" << "\n";
      if (exe.getReturnCode() == 0) {
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
  SnippetManager *snippet_manager = new SnippetManager();
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
    std::set<ASTNodeBase*> selection;
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

/**
 * Create selection. Output to outdir, one per json file.
 */
void create_selection(fs::path indir, fs::path outdir, int num, int num_token) {
  if (fs::exists(outdir)) fs::remove_all(outdir);
  fs::create_directories(outdir);
  SourceManager *sourceManager = new SourceManager(indir);
  for (int i=0;i<num;i++) {
    std::set<ASTNodeBase*> selection;
    fs::path file = outdir / (std::to_string(i) + ".json");
    std::ofstream os;
    os.open(file.string().c_str());
    assert(os.is_open());
    selection = sourceManager->genRandSelFunc(num_token);
    sourceManager->dumpJsonSelection(selection, os);
    os.close();
  }
}

/**
 * \mainpage Helium Reference Manual
 *
 * You might want to browse the important classes. The modules page shows some grouped classes.
 */
int main(int argc, char* argv[]) {
  utils::seed_rand();
  fs::path user_home(getenv("HOME"));
  if (user_home.empty()) {
    std::cerr << "EE: HOME env is not set." << "\n";
    exit(1);
  }
  fs::path helium_home(getenv("HELIUM_HOME"));
  if (helium_home.empty()) {
    std::cerr << "EE: HELIUM_HOME is not set." << "\n";
    exit(1);
  }

  /* parse arguments */
  HeliumOptions::Instance()->ParseCommandLine(argc, argv);
  HeliumOptions::Instance()->ParseConfigFile((helium_home / "helium.conf").string());

  if (HeliumOptions::Instance()->Has("help")) {
    HeliumOptions::Instance()->PrintHelp();
    exit(0);
  }

  fs::path indir = HeliumOptions::Instance()->GetString("target");
  fs::path outdir = HeliumOptions::Instance()->GetString("output");
  // absolute path
  indir = fs::canonical(indir);
  outdir = fs::canonical(outdir);

  if (HeliumOptions::Instance()->Has("preprocess")) {
    preprocess(indir, outdir);
    exit(0);
  }
  if (HeliumOptions::Instance()->Has("create-snippet")) {
    std::cout << "== Creating snippe ts, deps, outers ..." << "\n";
    SnippetManager *snippet_manager = new SnippetManager();
    snippet_manager->traverseDir(indir);
    // this actually is a json file
    snippet_manager->saveJson(outdir);
    exit(0);
  }
  if (HeliumOptions::Instance()->Has("create-selection")) {
    int num = HeliumOptions::Instance()->GetInt("sel-num");
    int num_token = HeliumOptions::Instance()->GetInt("sel-tok");
    create_selection(indir, outdir, num, num_token);
    exit(0);
  }
  if (HeliumOptions::Instance()->Has("create-include-dep")) {
    IncludeManager includeManager;
    includeManager.parse(indir);
    std::ofstream ofs(outdir.string());
    includeManager.dump(ofs);
    exit(0);
  }
  if (HeliumOptions::Instance()->Has("run")) {
    fs::path selection = HeliumOptions::Instance()->GetString("selection");
    fs::path snippet = HeliumOptions::Instance()->GetString("snippet");
    fs::path include_dep = HeliumOptions::Instance()->GetString("include-dep");
    SnippetManager *snip_manager = new SnippetManager();
    snip_manager->loadJson(snippet);
    snip_manager->processAfterLoad();
    IncludeManager *inc_manager = new IncludeManager();
    inc_manager->load(include_dep);
    LibraryManager *lib_manager = new LibraryManager();
    std::vector<std::string> v = HeliumOptions::Instance()->GetStringVector("header-config-json");
    for (std::string vi : v) {
      // 1. escape ~
      // 2. resolve sym link
      lib_manager->parse(fs::canonical(utils::escape_tide(vi)));
    }
    run_on_selection(indir, outdir, selection, snip_manager, inc_manager, lib_manager);
    exit(0);
  }
  std::cout << "[main] End Of Helium" << "\n";
  return 0;
}
