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

static void
create_tagfile(const std::string& folder, const std::string& file) {
  assert(utils::exists(folder));
  // use full path because I want to have the full path in tag file
  std::string full_path = utils::full_path(folder);
  std::string cmd = "ctags -f ";
  cmd += file;
  cmd += " --languages=c,c++ -n --c-kinds=+x --exclude=heium_result -R ";
  cmd += full_path;
  // std::cout << cmd  << "\n";
  // std::system(cmd.c_str());
  int status;
  utils::exec(cmd.c_str(), &status);
  if (status != 0) {
    std::cerr << "create tagfile failed" << "\n";
    std::cerr << cmd << "\n";
    exit(1);
  }
}

// void check_light_utilities() {
//   if (HeliumOptions::Instance()->Has("print-config")) {
//     std::cout << "DEPRECATED print-config"  << "\n";
//     exit(0);
//   }
//   if (HeliumOptions::Instance()->Has("resolve-system-type")) {
//     std::string type = HeliumOptions::Instance()->GetString("resolve-system-type");
//     std::string output = SystemResolver::Instance()->ResolveType(type);
//     std::cout << output  << "\n";
//     exit(0);
//   }
// }

// void check_target_folder(std::string folder) {
//   if (folder.empty()) {
//     HeliumOptions::Instance()->PrintHelp();
//     exit(1);
//   }
//   if (utils::is_dir(folder)) {
//     while (folder.back() == '/') folder.pop_back();
//   } else if (utils::is_file(folder)) {
//   } else {
//     std::cerr<<"target folder: " << folder << " does not exists." <<'\n';
//     assert(false);
//   }
// }

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

void load_snippet_db(std::string snippet_db_folder) {
  // std::string snippet_db_folder = HeliumOptions::Instance()->GetString("snippet-db-folder");
  if (snippet_db_folder.empty()) {
    if (HeliumOptions::Instance()->Has("verbose")) {
      std::cout
        << "Using default snippet folder: './snippets/'."
        << "If not desired, set via '-s' option."  << "\n";
    }
    snippet_db_folder = "snippets";
    // assert(false);
  }
  if (!utils::exists(snippet_db_folder)) {
    std::cout << "EE: Snippet folder " << snippet_db_folder << " does not exist."  << "\n";
    exit(1);
  }
  SnippetDB::Instance()->Load(snippet_db_folder);
}

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








/********************************
 * New staff
 *******************************/

void create_src(fs::path target, fs::path target_cache_dir) {
  fs::path src = target_cache_dir / "src";
  if (fs::exists(src)) fs::remove_all(src);
  fs::create_directories(src);
  // copy only source files. Keep directory structure
  fs::recursive_directory_iterator it(target), eod;
  BOOST_FOREACH (fs::path const & p, std::make_pair(it, eod)) {
    if (is_regular_file(p)) {
      if (p.extension() == ".c" || p.extension() == ".h") {
        fs::path to = target_cache_dir / "src" / fs::relative(p, target);
        fs::create_directories(to.parent_path());
        fs::copy_file(p, target_cache_dir / "src" / fs::relative(p, target));}}}}

/**
 * 1. create cpp folder
 * 2. preprocess src and put into cpp folder
 * 3. post-process cpp files
 */
void create_cpp(fs::path target_cache_dir) {
  fs::path cpp = target_cache_dir / "cpp";
  fs::path src = target_cache_dir / "src";
  if (fs::exists(cpp)) fs::remove_all(cpp);
  fs::create_directories(cpp);
  fs::recursive_directory_iterator it(src), eod;
  vector<fs::path> dirs;
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (fs::is_directory(p)) {
      dirs.push_back(p);
    }
  }

  std::string include_cmd;
  for (auto &p : dirs) {
    include_cmd += "-I" + p.string() + " ";
  }

  std::string cpp_cmd = "clang -E " + include_cmd;
    
  it = fs::recursive_directory_iterator(target_cache_dir / "src");
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (fs::is_regular_file(p)) {
      fs::path to = target_cache_dir / "cpp" / fs::relative(p, target_cache_dir / "src");
      fs::create_directories(to.parent_path());
      // redirect the clang preprocessor stderr to /dev/null
      std::string cmd = cpp_cmd + " " + p.string()
        + " >> " + to.string() + " 2>/dev/null";
      utils::new_exec(cmd.c_str());
    }
  }

  // remove extra things added by preprocessor using include files
  // std::cout << "Removing extra for cpp-ed file .." << "\n";
  it = fs::recursive_directory_iterator(target_cache_dir / "cpp");
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (fs::is_regular_file(p)) {
      std::ifstream is;
      is.open(p.string());
      std::string code;
      if (is.is_open()) {
        std::string line;
        std::string output;
        bool b = false;
        while(getline(is, line)) {
          if (!line.empty() && line[0] == '#') {
            // this might be a line marker
            vector<string> v = utils::split(line);
            if (v.size() < 3) continue;
            string filename = v[2];
            if (filename.empty() || filename[0] != '"' || filename.back() != '"') continue;
            filename = filename.substr(1);
            filename.pop_back();
            fs::path file(filename);
            fs::path newp = fs::relative(p, target_cache_dir / "cpp");
            fs::path newfile = fs::relative(file, target_cache_dir / "src");
            if (newfile.string() == newp.string()) {
              b = true;
              // I don't need the line marker line, because clang will complain when process it
              // output += line + "\n";
            } else {
              b = false;
            }
          } else {
            if (b) {
              output += line + "\n";
            }
          }
        }
        is.close();
        // output to the same file
        utils::write_file(p.string(), output);
      }
    }
  }
}

/**
 * create tagfile in target cache folder
 */
void create_tagfile(fs::path target_cache_dir) {
  // create tag file
  std::cout << "Creating tagfile .." << "\n";
  fs::path tagfile = target_cache_dir / "tagfile";
  fs::path cppfolder = target_cache_dir / "cpp";
  if (fs::exists(tagfile)) fs::remove(tagfile);
  create_tagfile(cppfolder.string(), tagfile.string());
}

/**
 * Create clang snippet database for snippets
 */
void create_clang_snippet(fs::path target_cache_dir) {
  std::cout << "Creating clang snippet .." << "\n";
  clangSnippetRun(target_cache_dir / "cpp");
  clangSnippetCreateDb(target_cache_dir / "clangSnippet.db");
  clangSnippetLoadDb(target_cache_dir / "clangSnippet.db");
  clangSnippetInsertDb();
}

/**
 * create general snippet db
 */
void create_snippet_db(fs::path target_cache_dir) {
  std::cout << "Creating snippet.db .." << "\n";
  fs::path tagfile = target_cache_dir/"tagfile";
  fs::path snippet_folder = target_cache_dir/"snippet";
  // // this will call srcml
  // SnippetDB::Instance()->Create(tagfile.string(), snippet_folder.string());
  // to create snippet db, we need
  // - tagfile
  // - output db file
  // - code folder
  // clang db folder
  clangSnippetLoadDb(target_cache_dir / "clangSnippet.db");
  SnippetDB::Instance()->CreateV2(target_cache_dir);
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

  fs::path systag = helium_home / "systype.tags";
  if (!fs::exists(systag)) {
    if (HeliumOptions::Instance()->Has("setup")) {
      // run ctags to create tags
      std::cout << "Creating ~/systype.tags" << "\n";
      std::string cmd = "ctags -f " + systag.string() +
        " --exclude=boost"
        " --exclude=llvm"
        " --exclude=c++"
        " --exclude=linux"
        " --exclude=xcb"
        " --exclude=X11"
        " --exclude=openssl"
        " --exclude=xorg"
        " -R /usr/include/ /usr/local/include";
      std::cout << "Running " << cmd << "\n";
      utils::exec(cmd.c_str());
      std::cout << "Done" << "\n";
      exit(0);
    } else {
      std::cout << "No systype.tags found. Run helium --setup first." << "\n";
      exit(1);
    }
  }
  SystemResolver::Instance()->Load(systag.string());
  
  // Reading target folder. This is the parameter
  std::string target_str = HeliumOptions::Instance()->GetString("target");
  target_str = utils::escape_tide(target_str);
  fs::path target(target_str);
  if (!fs::exists(target)) {
    std::cerr << "EE: target folder or file " << target.string() << " does not exist." << "\n";
    exit(1);}

  // change relative to absolute
  std::string target_cache_dir_name = fs::canonical(target).string();
  std::replace(target_cache_dir_name.begin(), target_cache_dir_name.end(), '/', '_');
  fs::path target_cache_dir(helium_home / "cache" / target_cache_dir_name);



  // (HEBI: Create Cache)


  if (HeliumOptions::Instance()->Has("create-cache")) {
    if (fs::exists(target_cache_dir)) fs::remove_all(target_cache_dir);
    fs::create_directories(target_cache_dir);
    create_src(target, target_cache_dir);
    create_cpp(target_cache_dir);
    create_tagfile(target_cache_dir);
    create_clang_snippet(target_cache_dir);
    create_snippet_db(target_cache_dir);
    exit(0);
  }
  if (HeliumOptions::Instance()->Has("create-cpp")) {
    create_src(target, target_cache_dir);
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

  // load_tagfile((target_cache_dir / "cpp").string());
  ctags_load((target_cache_dir / "tags").string());
  load_snippet_db((target_cache_dir / "snippet").string());
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


  if (HeliumOptions::Instance()->Has("tokenize")) {
    // get the target. we need to check if the target exists in the cache
    // - if not exist, prompt to cache it and exit
    // - if exist, do the work!
    // Do the work by scanning through all the source files in cache/NAME/cpp folder in alphabet order
    // this order can be altered later, maybe to support keep the directory hierarchy of source files to allow
    // source files of same name, but it is important to keep in mind that the order should be the same
    // for all the services that use the IDs of tokens.
    //
    // The tokens must also contains

    // produce a tokens.db in cache/XXX folder
    SourceManager *source_manager = new SourceManager(target_cache_dir / "cpp");
    source_manager->dumpASTs();
    exit(0);
  }

  if (HeliumOptions::Instance()->Has("selection")) {
    fs::path sel = HeliumOptions::Instance()->GetString("selection");
    // now we got ids, and we can start to run Helium!
    SourceManager *sourceManager = new SourceManager(target_cache_dir / "cpp");
    std::set<v2::ASTNodeBase*> selection = sourceManager->loadSelection(sel);
    sourceManager->select(selection);
    std::cout << "Run grammar patching on " << selection.size() << " selected tokens .." << "\n";
    std::set<v2::ASTNodeBase*> patch = sourceManager->grammarPatch();
    std::cout << "Done. Patch size: " << patch.size() << "\n";

    // analyze the distribution and print to std::cout
    sourceManager->analyzeDistribution(selection, patch, std::cout);
    
    exit(0);
  }

  if (HeliumOptions::Instance()->Has("random-selection")) {
    // generate random selection
    // and output to standard output (can be load back)
    SourceManager *sourceManager = new SourceManager(target_cache_dir / "cpp");
    std::set<v2::ASTNodeBase*> selection = sourceManager->generateRandomSelection();
    sourceManager->dumpSelection(selection, std::cout);
  }


  // if (HeliumOptions::Instance()->Has("distribution")) {
  //   // analyze the distribution of a selection.
  //   fs::path sel_file = HeliumOptions::Instance()->GetString("distribution");
  //   SourceManager *sourceManager = new SourceManager(target_cache_dir / "cpp");
  //   std::set<v2::ASTNodeBase*> selection = sourceManager->loadSelection(sel);
  //   sourceManager->analyzeDitribution(selection, std::cout);
  // }


  std::cerr << "Specify tokenize or selection to run." << "\n";
  exit(1);















  


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
