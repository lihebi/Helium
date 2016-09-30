#include <iostream>
#include <cstdlib>

#include "workflow/helium.h"

#include "resolver/snippet_db.h"
#include "parser/cfg.h"
#include "parser/xml_doc_reader.h"

#include "failure_point.h"
#include "helium_options.h"
#include "point_of_interest.h"
#include "utils/fs_utils.h"

#include <boost/filesystem.hpp>

#include <gtest/gtest.h>

namespace fs = boost::filesystem;

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
  if (HeliumOptions::Instance()->Has("show-ast")) {
    if (!utils::file_exists(folder)) {
      std::cerr << "only works for a single file.\n";
      exit(1);
    }
    XMLDoc *doc = XMLDocReader::CreateDocFromFile(folder);
    XMLNodeList func_nodes = find_nodes(*doc, NK_Function);
    for (XMLNode func : func_nodes) {
      AST ast(func);
      ast.Visualize2();
    }
    exit(0);
  }

  if (HeliumOptions::Instance()->Has("show-cfg")) {
    if (!utils::file_exists(folder)) {
      std::cerr << "only works for a single file.\n";
      exit(1);
    }
    XMLDoc *doc = XMLDocReader::CreateDocFromFile(folder);
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


  if (HeliumOptions::Instance()->Has("show-segments")) {
    assert(false);
    for (auto it=files.begin();it!=files.end();it++) {
      Reader reader(*it);
      // std::cout << "Segment count: " << reader.GetSegmentCount() << "\n";
      reader.PrintSegments();
    }
    exit(0);
  }
  if (HeliumOptions::Instance()->Has("show-segment-info")) {
    assert(false);
    for (auto it=files.begin();it!=files.end();it++) {
      Reader reader(*it);
    }
    exit(0);
  }
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



FailurePoint *load_failure_point() {
  std::vector<std::string> files = get_c_files(HeliumOptions::Instance()->GetString("folder"));
  if (HeliumOptions::Instance()->Has("poi")) {
    std::string poi_file = HeliumOptions::Instance()->GetString("poi");
    FailurePoint *fp = FailurePointFactory::CreateFailurePoint(poi_file);

    // find this file
    for (auto it=files.begin();it!=files.end();it++) {
      std::string filename = *it;
      if (filename.find(fp->GetFilename()) != std::string::npos) {
        return fp;
      } else {
        return NULL;
      }
    }
  } else if (HeliumOptions::Instance()->Has("whole-poi")) {
    std::string whole_poi_file = HeliumOptions::Instance()->GetString("whole-poi");
    if (!HeliumOptions::Instance()->Has("benchmark")) {
      std::cerr << "EE: benchmark name must be set (-b)"
                << "in order to use whole poi" << "\n";
      exit(1);
    }
    std::string benchmark = HeliumOptions::Instance()->GetString("benchmark");
    FailurePoint *fp = FailurePointFactory::CreateFailurePoint(whole_poi_file, benchmark);
    for (auto it=files.begin();it!=files.end();it++) {
      std::string filename = *it;
      if (filename.find(fp->GetFilename()) != std::string::npos) {
        return fp;
      } else {
        return NULL;
      }
    }
  }
  std::cerr << "EE: You should provide POI file."  << "\n";
  return NULL;
}







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
    exit(0);
  }

  
  // target folder
  std::string folder = HeliumOptions::Instance()->GetString("folder");
  folder = utils::escape_tide(folder);
  if (!fs::exists(folder)) {
    std::cerr << "EE: target folder " << folder << " does not exist." << "\n";
    exit(1);
  }
  std::string helium_home = HeliumOptions::Instance()->GetString("helium-home");
  SystemResolver::Instance()->Load(helium_home + "/systype.tags");


  
  // check_light_utilities();
  // check_target_folder(cpped.string());
  if (HeliumOptions::Instance()->Has("create-tagfile")) {
    create_tagfile(folder, "tags");
    exit(0);
  }

  if (HeliumOptions::Instance()->Has("create-snippet-db")) {
    std::string tmpdir = utils::create_tmp_dir();
    std::string tagfile = tmpdir+"/tags";
    create_tagfile(folder, tmpdir+"/tags");
    // std::cout << "created tagfile: " << tagfile  << "\n";
    SnippetDB::Instance()->Create(tagfile, "snippets");
    exit(0);
  }
  // create_utilities(folder);


  // the folder argument contains:
  // 1. cpped
  // 2. src
  // 3. snippets
  fs::path target(folder);
  fs::path cpped = target / "cpped";
  fs::path src = target / "src";
  fs::path snippets = target / "snippets";

  if (!fs::exists(cpped) || !fs::exists(src) || !fs::exists(snippets)) {
    std::cerr << "EE: cpped src snippets folders must exist in " << folder << "\n";
    exit(1);
  }

  if (HeliumOptions::Instance()->GetBool("print-benchmark-name")) {
    std::cout << "Benchmark Name: " << target.filename().string() << "\n";
  }

  load_tagfile(cpped.string());
  load_snippet_db(snippets.string());
  load_header_resolver(src.string());
  // load_slice();

  print_utilities(cpped.string());

  // FailurePoint *fp = load_failure_point();
  // if (!fp) {
  //   std::cerr << "EE: error loading failure point"  << "\n";
  //   exit(1);
  // }
  // Helium helium(fp);
  std::string poi_file = HeliumOptions::Instance()->GetString("poi-file");
  std::vector<PointOfInterest*> pois = create_point_of_interest(target.string(), poi_file);

  std::cout << "Number of point of interest: " << pois.size() << "\n";


  for (PointOfInterest *poi : pois) {
    Helium helium(poi);
  }

  std::cout << "End of Helium" << "\n";


  return 0;
}
