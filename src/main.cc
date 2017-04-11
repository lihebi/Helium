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


void create_utilities(std::string folder) {
  if (HeliumOptions::Instance()->Has("create-tagfile")) {
    std::string output_file = HeliumOptions::Instance()->GetString("output");
    if (output_file.empty()) output_file = "tags";
    create_tagfile(folder, output_file);
    exit(0);
  }

  if (HeliumOptions::Instance()->Has("create-snippet-db")) {
    std::string output_folder;
    if (HeliumOptions::Instance()->Has("output")) {
      output_folder = HeliumOptions::Instance()->GetString("output");
    }
    if (output_folder.empty()) output_folder = "snippets";

    if (fs::exists(output_folder)) {
      std::cerr << "Folder " << output_folder << " exists. Remove it before this command." << "\n";
      exit(1);
    }

    
    std::string tagfile;
    if (HeliumOptions::Instance()->Has("tagfile")) {
      tagfile = HeliumOptions::Instance()->GetString("tagfile");
    }
    std::string tmpdir = utils::create_tmp_dir();
    create_tagfile(folder, tmpdir+"/tags");
    tagfile = tmpdir+"/tags";
    std::cout << "created tagfile: " << tagfile  << "\n";
    SnippetDB::Instance()->Create(tagfile, output_folder);
    exit(0);
  }
}

std::vector<std::string> get_c_files(std::string folder) {
  std::vector<std::string> ret;
  /* get ret in target folder */
  if (utils::is_dir(folder)) {
    utils::get_files_by_extension(folder, ret, "c");
  } else {
    // this is just a file.
    ret.push_back(folder);
  }
  return ret;
}

void print_utilities(std::string folder) {

  if (HeliumOptions::Instance()->Has("show-callgraph")) {
    SnippetDB::Instance()->PrintCG();
    exit(0);
  }
  /**
   * Print src meta.
   * 0. check headers
   * 1. headers used
   * 2. header dependence
   */
  if (HeliumOptions::Instance()->Has("show-meta")) {
    std::cout << "== Helium Meta Data Printer =="  << "\n";
    std::cout << "== Headers"  << "\n";
    // 0
    SystemResolver::check_headers();
    // 1
    // std::string header = SystemResolver::Instance()->GetHeaders();
    // std::cout << header  << "\n";
    // std::cout << "== Headers available on the machine"  << "\n";
    std::set<std::string> avail_headers = SystemResolver::Instance()->GetAvailableHeaders();
    // for (std::string s : avail_headers) {
    //   std::cout << s  << "\n";
    // }
    std::cout << "== Headers used in project"  << "\n";
    std::cout  << "\t";
    std::set<std::string> used_headers = HeaderResolver::Instance()->GetUsedHeaders();
    for (std::string s : used_headers) {
      std::cout << s  << " ";
    }
    std::cout  << "\n";
    // 2
    std::cout << "== Header Dependence"  << "\n";
    // HeaderDep::Instance()->Dump();
    HeaderResolver::Instance()->DumpDeps();
    std::cout << "== Final headers included:"  << "\n";
    std::cout << "\t";
    for (std::string s : avail_headers) {
      if (used_headers.count(s) == 1) {
        std::cout << s  << " ";
      }
    }
    std::cout  << "\n";
    exit(0);
  }

  std::vector<std::string> files = get_c_files(folder);
}

void load_tagfile(std::string folder) {
  if (HeliumOptions::Instance()->Has("tagfile")) {
    std::string tagfile = HeliumOptions::Instance()->GetString("tagfile");
    ctags_load(tagfile);
  } else {
    std::string tmpdir = utils::create_tmp_dir();
    create_tagfile(folder, tmpdir+"/tags");
    std::string tagfile = tmpdir+"/tags";
    ctags_load(tagfile);
  }
}

// void load_snippet_db(std::string snippet_db_folder) {
//   // std::string snippet_db_folder = HeliumOptions::Instance()->GetString("snippet-db-folder");
//   if (snippet_db_folder.empty()) {
//     if (HeliumOptions::Instance()->Has("verbose")) {
//       std::cout
//         << "Using default snippet folder: './snippets/'."
//         << "If not desired, set via '-s' option."  << "\n";
//     }
//     snippet_db_folder = "snippets";
//     // assert(false);
//   }
//   if (!utils::exists(snippet_db_folder)) {
//     std::cout << "EE: Snippet folder " << snippet_db_folder << " does not exist."  << "\n";
//     exit(1);
//   }
//   SnippetDB::Instance()->Load(snippet_db_folder);
// }

void load_header_resolver(std::string src_folder) {
  // HeaderResolver::Instance()->Load(folder);
  // std::string src_folder = HeliumOptions::Instance()->GetString("src-folder");
  if (src_folder.empty()) {
    if (HeliumOptions::Instance()->Has("verbose")) {
      std::cerr
        << "Using default src folder: 'src'."
        << "If not desired, set via '-c' option."  << "\n";
    }
    src_folder = "src";
  }
  if (!utils::exists(src_folder)) {
    std::cerr << "EE: src folder " << src_folder << " does not exist."  << "\n";
    exit(1);
  }
  HeaderResolver::Instance()->Load(src_folder);
}

void load_slice() {
  if (HeliumOptions::Instance()->Has("slice-file")) {
    std::string slice_file = HeliumOptions::Instance()->GetString("slice-file");
    SimpleSlice::Instance()->SetSliceFile(slice_file);
  }
}


int deprecated_show_instrument_code() {
  std::string code = HeliumOptions::Instance()->GetString("show-instrument-code");
  if (code.empty()) {
    std::cerr << "EE: Code is empty." << "\n";
    return 1;
  }
  if (code.back() != ';') {
    std::cerr << "EE: code must be a decl_stmt, must end with semicolon" << "\n";
    return 1;
  }
  XMLDoc *doc = XMLDocReader::CreateDocFromString(code, "");
  XMLNode decl_node = find_first_node_bfs(doc->document_element(), "decl");
  Decl *decl = DeclFactory::CreateDecl(decl_node);
  Type *type = decl->GetType();
  std::string var = decl->GetName();
  std::string output = type->GetOutputCode(var);
  std::string input = type->GetInputCode(var);
  std::cout << "// Output:" << "\n";
  std::cout << output << "\n";
  std::cout << "// Input:" << "\n";
  std::cout << input << "\n";
  return 0;
}


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

/**
 * helium_target_name : the name helium represents a benchmark
 * _home_hebi_XXX_name
 */
void helium_run(fs::path helium_home, fs::path target, std::string helium_target_name, std::vector<fs::path> sels) {
  // std::cout << "Helium Running .." << "\n";
  fs::path target_cache_dir = helium_home / "cache" / helium_target_name;
  // fs::path target_sel_dir = helium_home / "sel" / helium_target_name;
  // fs::path target_sel_dir = target_cache_dir / "sel";
  fs::path gen_program_dir = target_cache_dir / "gen";
  if (!fs::exists(gen_program_dir)) fs::create_directories(gen_program_dir);

  SourceManager *sourceManager = new SourceManager(target_cache_dir / "cpp");
  for (fs::path sel_file : sels) {

    // DEBUG
    // sourceManager->dumpASTs();
      
    std::set<v2::ASTNodeBase*> sel = sourceManager->loadSelection(sel_file);
    std::cout << "---------------------" << "\n";
    std::cout << "[main] Rerun this by: helium " << target.string() << " --sel " << sel_file.string() << "\n";
    std::cout << "[main] Selected " << sel.size() << " tokens on Selection file " << sel_file.string() << "\n";
    // dumping
    for (v2::ASTNodeBase *node : sel) {
      node->dump(std::cout);
    }
    std::cout << "\n";
      
    // std::cout << "[main] Distribution:" << "\n";
    // sourceManager->analyzeDistribution(sel, {}, std::cout);

    std::cout << "[main] Doing Grammar Patching .." << "\n";

    // (HEBI: Grammar Patch)
    sel = sourceManager->grammarPatch(sel);
    // std::cout << "1: ";
    // for (v2::ASTNodeBase *node : sel) {node->dump(std::cout);}
    // std::cout << "\n";

    // (HEBI: Def-use)
    sel = sourceManager->defUse(sel);
    // std::cout << "2: ";
    // for (v2::ASTNodeBase *node : sel) {node->dump(std::cout);}
    // std::cout << "\n";

    // (HEBI: Grammar Patch)
    // I might want to do another grammar patching in case def use breaks it
    sel = sourceManager->grammarPatch(sel);
    // std::cout << "3: ";
    // for (v2::ASTNodeBase *node : sel) {node->dump(std::cout);}
    // std::cout << "\n";
      
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
    } else {
      std::cout << utils::RED << "[main] Compile Failure ..." << utils::RESET << "\n";
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

  if (HeliumOptions::Instance()->Has("dummy-loop")) {
    std::cout << "run dummy loop" << "\n";
    for (int i=0;i<1000;i++) {
      for (int j=0;j<1000;j++) {
        for (int k=0;k<1000;k++) {
          // i+j+k;
        }
      }
    }
    exit(0);
  }

  if (HeliumOptions::Instance()->Has("help")) {
    HeliumOptions::Instance()->PrintHelp();
    exit(0);}

  if (HeliumOptions::Instance()->Has("show-instrument-code")) {
    exit(deprecated_show_instrument_code());}

  fs::path user_home(getenv("HOME"));
  fs::path helium_home = user_home / ".helium.d";
  if (!fs::exists(helium_home)) {
    fs::create_directory(helium_home);}
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
    HeaderManager::Instance()->addConf(helium_home / "etc" / "system.conf");
    HeaderManager::Instance()->addConf(helium_home / "etc" / "third-party.conf");
    HeaderManager::Instance()->dump(std::cout);
    exit(0);
  }

  

  // fs::path systag = helium_home / "systype.tags";
  // if (!fs::exists(systag)) {
  //   if (HeliumOptions::Instance()->Has("setup")) {
  //     // run ctags to create tags
  //     std::cout << "Creating ~/systype.tags" << "\n";
  //     std::string cmd = "ctags -f " + systag.string() +
  //       " --exclude=boost"
  //       " --exclude=llvm"
  //       " --exclude=c++"
  //       " --exclude=linux"
  //       " --exclude=xcb"
  //       " --exclude=X11"
  //       " --exclude=openssl"
  //       " --exclude=xorg"
  //       " -R /usr/include/ /usr/local/include";
  //     std::cout << "Running " << cmd << "\n";
  //     utils::exec(cmd.c_str());
  //     std::cout << "Done" << "\n";
  //     exit(0);
  //   } else {
  //     std::cout << "No systype.tags found. Run helium --setup first." << "\n";
  //     exit(1);
  //   }
  // }
  // SystemResolver::Instance()->Load(systag.string());
  
  // Reading target folder. This is the parameter
  std::string target_str = HeliumOptions::Instance()->GetString("target");
  target_str = utils::escape_tide(target_str);
  fs::path target(target_str);
  if (!fs::exists(target)) {
    std::cerr << "EE: target folder or file " << target.string() << " does not exist." << "\n";
    exit(1);}
  // target is absolute path
  target = fs::canonical(target);

  if (HeliumOptions::Instance()->Has("bench-info")) {
    /**
     * I'll use this
     * It will display
     * - local header dependencies
     * - all system headers used
     * - system headers used but not defined in conf files
     *   - found on system (together with the flags)
     *   - not found on system
     */
    HeaderManager::Instance()->addConf(helium_home / "etc" / "system.conf");
    HeaderManager::Instance()->addConf(helium_home / "etc" / "third-party.conf");
    HeaderManager::Instance()->parseBench(target);
    std::cout << "== Header Dependencies:" << "\n";
    std::map<std::string, std::set<std::string> > deps = HeaderManager::Instance()->getDeps();
    for (auto &m : deps) {
      std::cout << m.first << " ==> \n";
      for (auto s : m.second) {
        std::cout << "\t" << s << "\n";
      }
    }
    std::set<std::string> all = HeaderManager::Instance()->infoGetAllHeaders();
    std::set<std::string> allMissExist = HeaderManager::Instance()->infoGetAllMissedExistHeaders();
    std::set<std::string> allMissNonExist = HeaderManager::Instance()->infoGetAllMissedNonExistHeaders();
    std::cout << "== All System Headers used: " << "\n";
    for (std::string s : all) {
      std::cout << "\t" << s << "\n";
    }
    std::cout << "== System headers used but not captured by Helium:" << "\n";
    std::cout << "==   1. Found on system:" << "\n";
    for (std::string s : allMissExist) {
      std::cout << "\t" << s << "\n";
    }
    std::cout << "==   2. Not found on system:" << "\n";
    for (std::string s : allMissNonExist) {
      std::cout << "\t" << s << "\n";
    }
    
    exit(0);
  }
  

  // change relative to absolute
  std::string target_dir_name = fs::canonical(target).string();
  std::replace(target_dir_name.begin(), target_dir_name.end(), '/', '_');
  fs::path target_cache_dir(helium_home / "cache" / target_dir_name);
  // fs::path target_sel_dir(helium_home / "sel" / target_dir_name);
  fs::path target_sel_dir = target_cache_dir / "sel";



  // see if this project is blocked
  {
    std::ifstream ifs((helium_home / "etc" / "blacklist.conf").string());
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
      std::cout << "checking " << s << "\n";
      if (target_dir_name.find(s) != std::string::npos) {
        std::cout << "[main] This project is in blacklist. Skipped." << "\n";
        exit(0);
      }
    }
  }

  // (HEBI: Create Cache)


  if (HeliumOptions::Instance()->Has("create-cache")) {
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

    // output a flag file that indicate cache is created But when
    // invoking the following tools along after this, it will not be
    // deleted, so still marked valid
    // utils::write_file((target_cache_dir/"valid").string(), "");
    exit(0);
  }
  if (HeliumOptions::Instance()->Has("create-cpp")) {
    create_src(target, target_cache_dir, target_sel_dir);
    create_cpp(target_cache_dir);
    exit(0);
  }
  if (HeliumOptions::Instance()->Has("create-tagfile")) {
    create_tagfile(target_cache_dir);
    exit(0);
  }
  if (HeliumOptions::Instance()->Has("create-clang-snippet")) {
    create_clang_snippet(target_cache_dir);
    exit(0);
  }
  if (HeliumOptions::Instance()->Has("create-snippet")) {
    create_snippet_db(target_cache_dir);
    exit(0);
  }






  


  

  if (!fs::exists(target_cache_dir)) {
    std::cerr << "The benchmark is not processed."
              << "Run helium --create-cache /path/to/benchmark" << "\n";
    exit(1);}

  // remove load of snippet db
  // SnippetDB::Instance()->Load(target_cache_dir / "snippet.db", target_cache_dir / "code");
  load_header_resolver((target_cache_dir / "src").string());

  if (HeliumOptions::Instance()->Has("info")) {
    // print out information about the benchmark
    // - which cache file
    // - # of func
    // - # of tok
    // - LOC
    // - # of snippets
    std::cout << "Information about benchmark " << target.string() << "\n";
    std::cout << "Using cache: " << target_cache_dir.string() << "\n";
    std::cout << "# of files: " << " TODO 8/32" << "\n";
    std::cout << "# of snippets: TODO " << SnippetDB::Instance()->numOfSnippet() << "\n";
    std::cout << "# of tokens: " << "TODO 330" << "\n";
    std::cout << "LOC: " << "TODO 8k" << "\n";
    exit(0);
  }

  // DEPRECATED
  if (HeliumOptions::Instance()->Has("selection")) {
    fs::path sel = HeliumOptions::Instance()->GetString("selection");
    // now we got ids, and we can start to run Helium!
    SourceManager *sourceManager = new SourceManager(target_cache_dir / "cpp");
    std::set<v2::ASTNodeBase*> selection = sourceManager->loadSelection(sel);
    // DEPRECATED
    // sourceManager->select(selection);
    std::cout << "Run grammar patching on " << selection.size() << " selected tokens .." << "\n";
    // std::set<v2::ASTNodeBase*> patch = sourceManager->grammarPatch();
    // std::cout << "Done. Patch size: " << patch.size() << "\n";

    // analyze the distribution and print to std::cout
    // sourceManager->analyzeDistribution(selection, patch, std::cout);
    
    exit(0);
  }

  // DEPRECATED
  // if (HeliumOptions::Instance()->Has("random-selection")) {
  //   // generate random selection
  //   // and output to standard output (can be load back)
  //   SourceManager *sourceManager = new SourceManager(target_cache_dir / "cpp");
  //   std::set<v2::ASTNodeBase*> selection = sourceManager->generateRandomSelection();
  //   sourceManager->dumpSelection(selection, std::cout);
  // }

  if (HeliumOptions::Instance()->Has("create-selection")) {
    int sel_num = HeliumOptions::Instance()->GetInt("sel-num");
    int sel_tok = HeliumOptions::Instance()->GetInt("sel-tok");
    // create selection
    // create selection folder if not exist
    if (!fs::exists(target_sel_dir)) fs::create_directories(target_sel_dir);
    SourceManager *sourceManager = new SourceManager(target_cache_dir / "cpp");
    // create and overwrite random/ folder
    fs::remove_all(target_sel_dir / "random");
    fs::create_directories(target_sel_dir / "random");

    for (int i=0;i<sel_num;i++) {
      std::set<v2::ASTNodeBase*> selection;
      fs::path file = target_sel_dir / "random" / (std::to_string(i) + ".sel");
      std::ofstream os;
      os.open(file.string().c_str());
      assert(os.is_open());
      // selection = sourceManager->genRandSelSameFunc(1);
      selection = sourceManager->genRandSel(sel_tok);
      sourceManager->dumpSelection(selection, os);
      os.close();
      std::cout << "Selection file wrote to " << file.string() << "\n";
    }
    exit(0);
  }

  if (HeliumOptions::Instance()->Has("dump-ast")) {
    SourceManager *sourceManager = new SourceManager(target_cache_dir / "cpp");
    sourceManager->dumpASTs();
    exit(0);
  }

  std::cout << "[main] Loading snippet from " << target_cache_dir << "\n";
  assert(fs::exists(helium_home / "etc" / "system.conf"));
  assert(fs::exists(helium_home / "etc" / "third-party.conf"));
  HeaderManager::Instance()->addConf(helium_home / "etc" / "system.conf");
  HeaderManager::Instance()->addConf(helium_home / "etc" / "third-party.conf");
  // parse the dependence before load snippet
  HeaderManager::Instance()->parseBench(target);

  HeaderManager::Instance()->adjustDeps(target.string(), (target_cache_dir / "cpp").string());


  {
    std::vector<std::string> v = HeliumOptions::Instance()->GetStringVector("header-config-json");
    for (std::string vi : v) {
      HeaderManager::Instance()->jsonAddConf(vi);
    }
    // HeaderManager::Instance()->jsonAddConf(helium_home / "etc" / "system.json");
    // HeaderManager::Instance()->jsonAddConf(helium_home / "etc" / "third-party.json");

    // HeaderManager::Instance()->jsonAddConf("/home/hebi/github/benchmark/package/database/core.new.json");
    // HeaderManager::Instance()->jsonAddConf("/home/hebi/github/benchmark/package/database/extra.new.json");
    // HeaderManager::Instance()->jsonAddConf("/home/hebi/github/benchmark/package/database/community.new.json");
    // HeaderManager::Instance()->jsonAddConf("/home/hebi/github/benchmark/package/database/multilib.new.json");
    HeaderManager::Instance()->jsonParseBench(target);
    HeaderManager::Instance()->jsonResolve();
  }

  if (HeliumOptions::Instance()->Has("hebi")) {
    std::set<std::string> headers = HeaderManager::Instance()->jsonGetHeaders();
    std::set<std::string> flags = HeaderManager::Instance()->jsonGetFlags();
    std::cout << "Header: " << "\n";
    for (std::string header : headers) {
      std::cout << header << " ";
    }
    std::cout << "\n";
    std::cout << "Flag: " << "\n";
    for (std::string flag : flags) {
      std::cout << flag << " ";
    }
    std::cout << "\n";
    exit(0);
  }

  v2::GlobalSnippetManager::Instance()->loadJson(target_cache_dir / "snippets.json");
  // CAUTION Must process after load.
  v2::GlobalSnippetManager::Instance()->processAfterLoad();
  v2::GlobalSnippetManager::Instance()->dump(std::cout);


  // (HEBI: Running)
  // Put the selection file in specific folder, Helium will find them itself.

  std::cout << "[main] Running Helium on " << target.string() << " .." << "\n";
  // fs::path target_sel_dir = helium_home / "sel" / target_cache_dir_name;

  // get selection files
  std::vector<fs::path> sels;
  if (HeliumOptions::Instance()->Has("sel")) {
    fs::path sel_config = HeliumOptions::Instance()->GetString("sel");
    if (fs::is_regular_file(sel_config)) {
      sels.push_back(sel_config);
    }
    if (fs::is_directory(sel_config)) {
      fs::recursive_directory_iterator it(target_sel_dir), eod;
      BOOST_FOREACH (fs::path const & p, std::make_pair(it, eod)) {
        // must be .sel file
        if (is_regular_file(p) && p.extension() == ".sel") {
          sels.push_back(sel_config);
        }
      }
    }
  } else {
    fs::recursive_directory_iterator it(target_sel_dir), eod;
    BOOST_FOREACH (fs::path const & p, std::make_pair(it, eod)) {
      // must be .sel file
      if (is_regular_file(p) && p.extension() == ".sel") {
        sels.push_back(p);
      }
    }
  }

  helium_run(helium_home, target, target_dir_name, sels);

  std::cout << "[main] End Of Helium" << "\n";
  exit(0);












  


  // check if the snippet database has been generated
  

  // load_tagfile(cpped.string());
  // load_snippet_db(snippets.string());
  // load_header_resolver(src.string());
  // load_slice();

  
  // check_light_utilities();
  // check_target_folder(cpped.string());
  // if (HeliumOptions::Instance()->Has("create-tagfile")) {
  //   create_tagfile(target_str, "tags");
  //   exit(0);
  // }

  if (HeliumOptions::Instance()->Has("create-snippet-db")) {
    std::string tmpdir = utils::create_tmp_dir();
    std::string tagfile = tmpdir+"/tags";
    create_tagfile(target_str, tmpdir+"/tags");
    // std::cout << "created tagfile: " << tagfile  << "\n";
    SnippetDB::Instance()->Create(tagfile, "snippets");
    exit(0);
  }
  // create_utilities(folder);


  if (HeliumOptions::Instance()->Has("show-ast")) {
    if (!utils::file_exists(target_str)) {
      std::cerr << "only works for a single file.\n";
      exit(1);
    }
    XMLDoc *doc = XMLDocReader::CreateDocFromFile(target_str);
    XMLNodeList func_nodes = find_nodes(*doc, NK_Function);
    for (XMLNode func : func_nodes) {
      AST ast(func);
      ast.Visualize2();
    }
    exit(0);
  }

  if (HeliumOptions::Instance()->Has("show-cfg")) {
    if (!utils::file_exists(target_str)) {
      std::cerr << "only works for a single file.\n";
      exit(1);
    }
    XMLDoc *doc = XMLDocReader::CreateDocFromFile(target_str);
    XMLNodeList func_nodes = find_nodes(*doc, NK_Function);
    for (XMLNode func : func_nodes) {
      AST ast(func);
      ASTNode *root = ast.GetRoot(); 
      CFG *cfg = CFGFactory::CreateCFG(root);
      cfg->Visualize();
      delete cfg;
    }
    exit(0);
  }

  

  // if (!fs::exists(cpped) || !fs::exists(src) || !fs::exists(snippets)) {
  //   std::cerr << "EE: cpped src snippets folders must exist in " << folder << "\n";
  //   std::cout << cpped << "\n";
  //   std::cout << src << "\n";
  //   std::cout << snippets << "\n";
  //   exit(1);
  // }

  if (HeliumOptions::Instance()->GetBool("print-benchmark-name")) {
    std::cout << "Benchmark Name: " << target.filename().string() << "\n";
  }

  // load_tagfile(cpped.string());
  // load_snippet_db(snippets.string());
  // load_header_resolver(src.string());
  // load_slice();

  // print_utilities(cpped.string());

  // FailurePoint *fp = load_failure_point();
  // if (!fp) {
  //   std::cerr << "EE: error loading failure point"  << "\n";
  //   exit(1);
  // }
  // Helium helium(fp);
  std::string poi_file = HeliumOptions::Instance()->GetString("poi-file");
  poi_file = utils::escape_tide(poi_file);
  // std::vector<PointOfInterest*> pois = create_point_of_interest(target.string(), poi_file);
  std::vector<PointOfInterest*> pois = POIFactory::Create(target.string(), poi_file);

  std::cout << "Number of point of interest: " << pois.size() << "\n";


  int ct=0;
  int valid_poi_ct=0;
  int valid_poi_limit=HeliumOptions::Instance()->GetInt("valid-poi-limit");
  for (PointOfInterest *poi : pois) {
    std::cerr << "Current POI: " << ct << "\n";
    ct++;
    Helium helium(poi);
    if (helium.GetStatus() == HS_Success) {
      valid_poi_ct++;
    }
    if (valid_poi_limit>0 && valid_poi_ct > valid_poi_limit) {
      std::cerr << "Reach Valid POI Limit: " << valid_poi_limit << ". Breaking ..\n";
      break;
    }
  }

  std::cout << "End of Helium" << "\n";


  return 0;
}
