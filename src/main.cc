#include <iostream>
#include <cstdlib>

#include "helium/utils/XMLDocReader.h"

#include "helium/parser/Parser.h"

#include "helium/utils/HeliumOptions.h"
#include "helium/utils/FSUtils.h"
#include "helium/utils/Utils.h"

#include "helium/parser/SourceManager.h"

#include "helium/type/Cache.h"

#include "helium/type/Snippet.h"
#include "helium/type/SnippetManager.h"
#include "helium/type/SnippetAction.h"

#include "helium/utils/ThreadUtils.h"
#include "helium/utils/ColorUtils.h"
#include "helium/utils/StringUtils.h"
#include "helium/utils/RandUtils.h"
#include "helium/utils/FSUtils.h"
#include "helium/utils/Dot.h"


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
  // cmd += " 2>&1";
  ThreadExecutor exe(cmd);
  exe.run();
  // std::cout << "[main] do compile: stdout" << "\n" << exe.getStdOut() << "\n";
  // std::cout << "[main] do compile Stderr: " << exe.getStdErr() << "\n";
  if (exe.getReturnCode() == 0) {
    return true;
  } else {
    return false;
  }
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
  if (fs::exists(outdir)) fs::remove_all(outdir);
  fs::create_directories(outdir);
  SourceManager *source_man = new SourceManager();
  source_man->parse(indir);
  std::vector<fs::path> sels = get_selections(selection);
  std::cout << "[main] Running on " << sels.size() << " selections .." << "\n";

  std::vector<std::string> compile_suc, run_suc;
  for (fs::path sel_file : sels) {
    std::set<ASTNodeBase*> sel = source_man->loadJsonSelection(sel_file);
    // source_man->dumpDist(sel, std::cout);
    sel = source_man->grammarPatch(sel);
    // sel = source_man->defUse(sel);
    // I might want to do another grammar patching in case def use breaks it
    // sel = source_man->grammarPatch(sel);
    // Generate into folder and do compilation
    // use the sel filename as the folder
    fs::path gen_dir = outdir / sel_file.filename().stem();
    
    source_man->generate(sel, gen_dir, snip_manager, inc_manager, lib_manager);
    if (do_compile(gen_dir)) {
      // compile success
      compile_suc.push_back(sel_file.filename().string());
      
      std::string run_cmd = gen_dir.string() + "/a.out";
      ThreadExecutor exe(run_cmd);
      exe.setTimeoutSec(0.2);
      exe.run();
      if (exe.getReturnCode() == 0) {
        // run success
        run_suc.push_back(sel_file.filename().string());
      } else if (exe.isTimedOut()) {
        // timeout
      } else {
        // other reason to failure
      }
    } else {
      // compile failure
    }
  }
  // report build rate
  // I can also get this information from script
  // by checking if the folder contains a.out file
  // and if the folder contains helium-output.txt
  std::cout << "[main] Build Rate: " << compile_suc.size() << " / " << sels.size() << " = "
            << ((double)compile_suc.size() / sels.size() * 100) << "%" << "\n";
  if (!compile_suc.empty()) {
    std::cout << "[main] Run Rate: " << run_suc.size() << " / " << compile_suc.size() << " = "
              << (double)run_suc.size() / compile_suc.size() * 100 << "%" << "\n";
  }
}



/**
 * Create selection. Output to outdir, one per json file.
 */
void create_selection(fs::path indir, fs::path outdir, int num, int num_token) {
  if (fs::exists(outdir)) fs::remove_all(outdir);
  fs::create_directories(outdir);
  SourceManager *source_man = new SourceManager();
  source_man->parse(indir);
  for (int i=0;i<num;i++) {
    std::set<ASTNodeBase*> selection;
    fs::path file = outdir / (std::to_string(i) + ".json");
    std::ofstream os;
    os.open(file.string().c_str());
    assert(os.is_open());
    selection = source_man->genRandSelFunc(num_token);
    source_man->dumpJsonSelection(selection, os);
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
  // fs::path helium_home(getenv("HELIUM_HOME"));
  // if (helium_home.empty()) {
  //   std::cerr << "EE: HELIUM_HOME is not set." << "\n";
  //   exit(1);
  // }

  /* parse arguments */
  HeliumOptions *options = new HeliumOptions;
  options->ParseCommandLine(argc, argv);
  options->ParseConfigFile((user_home / ".helium.d" / "helium.conf").string());

  if (options->Has("help")) {
    options->PrintHelp();
    exit(0);
  }

  fs::path indir = options->GetString("target");
  // absolute path
  indir = fs::canonical(indir);


  if (options->Has("dump-cfg")) {
    fs::path outdir = options->GetString("output");
    if (!fs::is_regular(indir)) {
      std::cerr << indir << " is not a file." << "\n";
      exit(1);
    }
    if (fs::exists(outdir)) fs::remove_all(outdir);
    fs::create_directories(outdir);
    
    std::string file(indir.string());
    Parser *parser = new SrcMLParser();
    ASTContext *ast = parser->parse(file);
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    CFGBuilder builder;

    if (options->Has("cfg-no-decl")) {
      builder.addOption(CFG_NoDecl);
    }
    
    unit->accept(&builder);
    CFG *cfg = builder.getCFG();

    std::string ggx = cfg->getGgxString();
    utils::write_file(outdir / "whole.ggx", ggx);
    std::string dot = cfg->getDotString();
    utils::write_file(outdir / "whole.dot", dot);
    dot2png(outdir / "whole.dot", outdir / "whole.png");
    exit(0);
  }

  if (options->Has("dump-ast")) {
    if (!fs::is_regular(indir)) {
      std::cerr << indir << " is not a file." << "\n";
      exit(1);
    }
    // fs::path outdir = options->GetString("output");
    // if (fs::exists(outdir)) fs::remove_all(outdir);
    // fs::create_directories(outdir);
    
    Parser *parser = new ClangParser();
    ASTContext *ast = parser->parse(indir);
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    
    // std::ofstream ofs((outdir / "ast.lisp").string());
    Printer printer;
    unit->accept(&printer);
    std::string output = printer.getString();
    std::cout << output << "\n";
    // ofs.close();
    
    exit(0);
  }
  if (options->Has("preprocess")) {
    fs::path outdir = options->GetString("output");
    preprocess(indir, outdir);
    exit(0);
  }
  if (options->Has("create-selection")) {
    fs::path outdir = options->GetString("output");
    int num = options->GetInt("sel-num");
    int num_token = options->GetInt("sel-tok");
    create_selection(indir, outdir, num, num_token);
    exit(0);
  }
  if (options->Has("create-include-dep")) {
    fs::path outdir = options->GetString("output");
    IncludeManager inc_man;
    inc_man.parse(indir);
    std::ofstream ofs(outdir.string());
    inc_man.dump(ofs);
    ofs.close();
    exit(0);
  }
  if (options->Has("create-snippet")) {
    fs::path outdir = options->GetString("output");
    fs::path include_dep = options->GetString("include-dep");
    IncludeManager *inc_man = new IncludeManager();
    inc_man->load(include_dep);
    
    SnippetManager *snippet_manager = new SnippetManager();
    snippet_manager->parse(indir, inc_man);
    // this actually is a json file
    std::ofstream ofs(outdir.string());
    snippet_manager->dump(ofs);
    ofs.close();
    exit(0);
  }
  if (options->Has("run")) {
    fs::path outdir = options->GetString("output");
    fs::path selection = options->GetString("selection");
    fs::path snippet = options->GetString("snippet");
    fs::path include_dep = options->GetString("include-dep");
    SnippetManager *snip_manager = new SnippetManager();
    snip_manager->load(snippet);
    IncludeManager *inc_manager = new IncludeManager();
    inc_manager->load(include_dep);
    LibraryManager *lib_manager = new LibraryManager();
    std::vector<std::string> v = options->GetStringVector("header-config-json");
    for (std::string vi : v) {
      // 1. escape ~
      // 2. resolve sym link
      lib_manager->parse(fs::canonical(utils::escape_tide(vi)));
    }
    run_on_selection(indir, outdir, selection, snip_manager, inc_manager, lib_manager);
    exit(0);
  }
  if (options->Has("codegen")) {
    fs::path sel = options->GetString("selection");
    if (!fs::is_regular(sel)) {
      std::cerr << sel << " should be a regular json file, not a directory" << "\n";
      exit(1);
    }

    SourceManager *source_man = new SourceManager();
    source_man->parse(indir);
    source_man->loadJsonSelection(sel);
  }
  std::cout << "[main] End Of Helium" << "\n";
  return 0;
}
