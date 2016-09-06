#include "helium.h"
#include <string>
#include <iostream>

#include "reader.h"
#include "hebi.h"
#include "helium_utils.h"

#include "config/arg_parser.h"
#include "config/config.h"
#include "config/options.h"

#include "parser/xml_doc_reader.h"
#include "parser/xmlnode.h"
#include "parser/ast_node.h"
#include "parser/cfg.h"

#include "utils/utils.h"
#include "utils/dump.h"

#include "resolver/snippet.h"
#include "resolver/resolver.h"
#include "resolver/snippet_db.h"


#include <gtest/gtest.h>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
namespace fs = boost::filesystem;


using namespace utils;

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


Helium::Helium(int argc, char* argv[]) {
  /* load HELIUM_HOME */
  std::string helium_home = load_helium_home();
  /* parse arguments */
  ArgParser::Instance()->Set(argc, argv);

  /* load config */
  Config::Instance()->ParseFile(helium_home+"/helium.conf");
  Config::Instance()->Overwrite();
  if (ArgParser::Instance()->Has("print-config")) {
    std::cout << Config::Instance()->ToString() << "\n";
    exit(0);
  }

  SystemResolver::Instance()->Load(helium_home + "/systype.tags");

  if (ArgParser::Instance()->Has("resolve-system-type")) {
    std::string type = ArgParser::Instance()->GetString("resolve-system-type");
    std::string output = SystemResolver::Instance()->ResolveType(type);
    std::cout << output  << "\n";
    exit(0);
  }


  // target folder
  m_folder = ArgParser::Instance()->GetString("folder");
  if (m_folder.empty()) {
    ArgParser::Instance()->PrintHelp();
    exit(1);
  }
  if (utils::is_dir(m_folder)) {
    while (m_folder.back() == '/') m_folder.pop_back();
  } else if (utils::is_file(m_folder)) {
  } else {
    std::cerr<<"target folder: " << m_folder << " does not exists." <<'\n';
    assert(false);
  }

  /*******************************
   ** utilities
   *******************************/
  if (ArgParser::Instance()->Has("create-tagfile")) {
    std::string output_file = ArgParser::Instance()->GetString("output");
    if (output_file.empty()) output_file = "tags";
    create_tagfile(m_folder, output_file);
    exit(0);
  }

  if (ArgParser::Instance()->Has("create-snippet-db")) {
    std::string output_folder;
    if (ArgParser::Instance()->Has("output")) {
      output_folder = ArgParser::Instance()->GetString("output");
    }
    if (output_folder.empty()) output_folder = "snippets";
    std::string tagfile;
    if (ArgParser::Instance()->Has("tagfile")) {
      tagfile = ArgParser::Instance()->GetString("tagfile");
    }
    std::string tmpdir = utils::create_tmp_dir();
    create_tagfile(m_folder, tmpdir+"/tags");
    tagfile = tmpdir+"/tags";
    std::cout << "created tagfile: " << tagfile  << "\n";
    SnippetDB::Instance()->Create(tagfile, output_folder);
    exit(0);
  }
  // if (ArgParser::Instance()->Has("print-header-deps")) {
  //   HeaderResolver::Instance()->Load(m_folder);
  //   HeaderResolver::Instance()->Dump();
  //   exit(0);
  // }

  if (ArgParser::Instance()->Has("print-ast")) {
    if (!utils::file_exists(m_folder)) {
      std::cerr << "only works for a single file.\n";
      exit(1);
    }
    XMLDoc *doc = XMLDocReader::CreateDocFromFile(m_folder);
    XMLNodeList func_nodes = find_nodes(*doc, NK_Function);
    for (XMLNode func : func_nodes) {
      AST ast(func);
      ast.Visualize2();
    }
    exit(0);
  }

  if (ArgParser::Instance()->Has("print-cfg")) {
    if (!utils::file_exists(m_folder)) {
      std::cerr << "only works for a single file.\n";
      exit(1);
    }
    XMLDoc *doc = XMLDocReader::CreateDocFromFile(m_folder);
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
  
  /* load tag file */
  std::string tagfile = ArgParser::Instance()->GetString("tagfile");
  if (tagfile.empty()) {
    // ctags_load(m_folder + "/tags");
    // create tagfile
    // std::cout << "creating tag file ..."  << "\n";
    std::string tmpdir = utils::create_tmp_dir();
    create_tagfile(m_folder, tmpdir+"/tags");
    tagfile = tmpdir+"/tags";
    // create_tagfile(m_folder, "/tmp/helium.tags");
    // std::cout << "done"  << "\n";
    // ctags_load("/tmp/helium.tags");
    ctags_load(tagfile);
  } else {
    ctags_load(tagfile);
  }

  std::string snippet_db_folder = ArgParser::Instance()->GetString("snippet-db-folder");
  if (snippet_db_folder.empty()) {
    if (ArgParser::Instance()->Has("verbose")) {
      // std::cerr << "snippet database folder unset. Please use --snippet-db-folder option. --help for more details." << "\n";
      std::cout << "Using default snippet folder: './snippets/'. If not desired, set via '-s' option."  << "\n";
    }
    snippet_db_folder = "snippets";
    // assert(false);
  }
  if (!utils::exists(snippet_db_folder)) {
    std::cout << "EE: Snippet folder " << snippet_db_folder << " does not exist."  << "\n";
    exit(1);
  }
  SnippetDB::Instance()->Load(snippet_db_folder);


  if (ArgParser::Instance()->Has("print-callgraph")) {
    SnippetDB::Instance()->PrintCG();
    exit(0);
  }

  

  // HeaderResolver::Instance()->Load(m_folder);
  std::string src_folder = ArgParser::Instance()->GetString("src-folder");


  if (src_folder.empty()) {
    if (ArgParser::Instance()->Has("verbose")) {
      std::cerr << "Using default src folder: 'src'. If not desired, set via '-c' option."  << "\n";
    }
    src_folder = "src";
  }
  if (!utils::exists(src_folder)) {
    std::cerr << "EE: src folder " << src_folder << " does not exist."  << "\n";
    exit(1);
  }
  
  HeaderResolver::Instance()->Load(src_folder);
  /* load system tag file */


  /**
   * Print src meta.
   * 0. check headers
   * 1. headers used
   * 2. header dependence
   */
  if (ArgParser::Instance()->Has("print-meta")) {
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

  /* get files in target folder */
  if (utils::is_dir(m_folder)) {
    get_files_by_extension(m_folder, m_files, "c");
  } else {
    // this is just a file.
    m_files.push_back(m_folder);
  }

  /*******************************
   ** More advanced utils(needs to run some functionality of Helium)
   *******************************/

  if (ArgParser::Instance()->Has("print-segments")) {
    assert(false);
    for (auto it=m_files.begin();it!=m_files.end();it++) {
      Reader reader(*it);
      // std::cout << "Segment count: " << reader.GetSegmentCount() << "\n";
      reader.PrintSegments();
    }
    exit(0);
  }
  if (ArgParser::Instance()->Has("print-segment-info")) {
    assert(false);
    for (auto it=m_files.begin();it!=m_files.end();it++) {
      Reader reader(*it);
      // std::cout << "Segment count: " << reader.GetSegmentCount() << "\n";
      // std::cout << "segment size: " << reader.GetSegmentLOC() << "\n";
    }
    exit(0);
  }

  // if (ArgParser::Instance()->Has("slice")) {
  //   // use slicing as code selection method
  //   std::string slice_file = ArgParser::Instance()->GetString("slice");
  //   Reader::slice(slice_file, m_folder);
  //   std::cout << BuildRatePlotDump::Instance()->dump()  << "\n";
  //   exit(0);
  // }

  /**
   * TODO so many singletons are good practice? Maybe I need to make the Helium class singleton and accessible, and make these singletons its fields.
   */
  if (ArgParser::Instance()->Has("slice-file")) {
    std::string slice_file = ArgParser::Instance()->GetString("slice-file");
    SimpleSlice::Instance()->SetSliceFile(slice_file);
  }

  if (ArgParser::Instance()->Has("poi")) {
    m_poi_file = ArgParser::Instance()->GetString("poi");
  } else if (ArgParser::Instance()->Has("whole-poi")) {
    m_whole_poi = ArgParser::Instance()->GetString("whole-poi");
  } else {
    std::cerr << "EE: You should provide POI file."  << "\n";
    exit(1);
  }
  if (ArgParser::Instance()->Has("benchmark")) {
    m_benchmark = ArgParser::Instance()->GetString("benchmark");
  }
}
Helium::~Helium() {}

/**
 * Get segments based on config file.
 * this should be a config file with the filename:line_number format
 * the detailed formats:
 * a.c:504
 * TODO a.c:504-510
 * TODO to support function name
 * @return a.c -> 102,208
 */
std::map<std::string, std::vector<int> > parse_selection_conf(const std::string& file) {
  std::map<std::string, std::vector<int> > result;
  std::string content = utils::read_file(file);
  // delimiter by *space*
  std::vector<std::string> lines = utils::split(content);
  for (std::string &line : lines) {
    std::vector<std::string> tmp = utils::split(line, ':');
    assert(tmp.size() == 2 && "Config file error for code selection.");
    std::string f = tmp[0];
    std::string l_str = tmp[1];
    utils::trim(f);
    utils::trim(l_str);
    int l = atoi(l_str.c_str());
    if (result.find(f) == result.end()) {
      result[f] = std::vector<int>();
    }
    // FIXME value or reference?
    result[f].push_back(l);
  }
  return result;
}

TEST(helium_test_case, parse_selection_conf_test) {
  char tmp_dir[] = "/tmp/helium-test-temp.XXXXXX";
  char *result = mkdtemp(tmp_dir);
  ASSERT_TRUE(result != NULL);
  std::string dir = tmp_dir;
  std::string filename = dir+"/a.c";
  const char *code = R"prefix(

a.c:100
a.c:235
sub/dir/b.cpp:364

)prefix";
  utils::write_file(filename, code);

  std::map<std::string, std::vector<int> > m = parse_selection_conf(filename);
  ASSERT_EQ(m.size(), 2);
  std::vector<int> a = m["a.c"];
  ASSERT_EQ(a.size(), 2);
  EXPECT_EQ(a[0], 100);
  EXPECT_EQ(a[1], 235);
  std::vector<int> b = m["sub/dir/b.cpp"];
  ASSERT_EQ(b.size(), 1);
  EXPECT_EQ(b[0], 364);
}

int Helium::countFunction() {
  int ret=0;
  for (std::string file : m_files) {
    XMLDoc doc;
    utils::file2xml(file, doc);
    XMLNodeList nodes = find_nodes(doc, NK_Function);
    ret += nodes.size();
  }
  return ret;
}

POISpec parse_poi_file(std::string file) {
  print_trace("parse_poi_file");
  POISpec ret;
  // read the first line
  // split into three parts
  // simple filename, line number, type
  std::string content = utils::read_file(file);
  std::vector<std::string> sp = utils::split(content);
  assert(sp.size() > 2);
  ret.filename = sp[0];
  try {
    ret.linum = std::stoi(sp[1]);
  } catch (std::invalid_argument e) {
    std::cerr << "error parsing poi file line number"  << "\n";
    exit(1);
  }
  ret.type = sp[2];
  assert(ret.type == "loop" || ret.type == "stmt");
  std::cout << "POI: " << ret.filename << ":" << ret.linum << " " << ret.type  << "\n";
  return ret;
}

/**
 * Parse whole poi file.
 * TODO use a format table parser
 | benchmark                 | file               | line | type | bug-type |
 |---------------------------+--------------------+------+------+----------|
 | cabextract-1.2            | mszipd.c           |  353 | loop |          |
 */
POISpec parse_whole_poi_file(std::string file, std::string benchmark) {
  POISpec ret;
  std::string content = utils::read_file(file);
  std::vector<std::string> lines = utils::split(content, '\n');
  for (std::string line : lines) {
    std::vector<std::string> components = utils::split(line, '|');
    // FIXME the first is empty?
    // FIXME what is there's one empty in the middle?
    // FIXME in the end?
    if (components.size() < 6) {
      continue;
    }
    std::string bench = components[1];
    std::string file = components[2];
    std::string linum = components[3];
    std::string type = components[4];
    std::string bug_type = components[5];
    utils::trim(bench);
    utils::trim(file);
    utils::trim(linum);
    utils::trim(type);
    utils::trim(bug_type);
    // std::cout << bench << ":" << file << ":" << linum  << "\n";
    if (benchmark == bench) {
      std::cout << "found POI: " << file << ":" << linum << " " << type  << "\n";
      ret.filename = file;
      try {
        ret.linum = std::stoi(linum);
      } catch (std::invalid_argument e) {
        std::cout << "Error parsing poi file " << file << ": linum is not a number"  << "\n";
        exit(1);
      }
      ret.type = type;
      return ret;
    }
  }
  return ret;
}

TEST(HeliumTestCase, WholePOITest) {
  std::string s = "| xfd | fds | | xxfd | |";
  // the test shows: the first is empty, the last empty is omitted (in this case only one last empty string)
  std::vector<std::string> v = utils::split(s, '|');
  for (std::string s : v) {
    std::cout << "COMP: " << s  << "\n";
  }
}

void
Helium::Run() {
  print_trace("Helium::Run");
  /**
   * Print some meta data for the benchmark project.
   */
  std::cerr << "***** Helium Benchmark Meta *****"  << "\n";
  std::cerr << "** total file: " << m_files.size()  << "\n";
  // int func_count = countFunction();
  // std::cerr << "** totla function: " << func_count  << "\n";
  std::cerr << "*********************************" << "\n";
  // ExpASTDump::Instance()->file_count = m_files.size();
  // ExpASTDump::Instance()->func_count = func_count;
  // ExpASTDump::Instance()->benchmark = m_folder;
  // double t1 = utils::get_time();
  /**
   * Code selection method is given as a file, so this is a config file, contains the <file:line> format
   */
  // if (utils::file_exists(Config::Instance()->GetString("code-selection"))) {
    // code selection method is a config file, we only need to check those files in the config.
    // std::map<std::string, std::vector<int> > conf = parse_selection_conf(Config::Instance()->GetString("code-selection"));
    // for (auto c : conf) {
    //   std::string filename = m_folder + "/" + c.first;
    //   if (utils::file_exists(filename)) {
    //     std::cerr << "processing: " << filename << " ...\n";
    //     Reader reader(filename, c.second);
    //     reader.PrintSegments();
    //     // reader.Read();
    //   }
    // }
  // } else {
  // }

  if (!m_poi_file.empty()) {
    assert(utils::file_exists(m_poi_file));
    // filename, line number, typ
    POISpec poi = parse_poi_file(m_poi_file);
    if (poi.filename.empty()) return;
    // find this file
    for (auto it=m_files.begin();it!=m_files.end();it++) {
      std::string filename = *it;
      if (filename.find(poi.filename) != std::string::npos) {
        // Reader reader(filename, poi);
        hebi(filename, poi);
      }
    }
    // find this line in "cpped" file, by line marker
  } else if (!m_whole_poi.empty()) {
    if (m_benchmark.empty()) {
      std::cerr << "EE: benchmark name must be set (--benchmark, -b) in order to use whole poi" << "\n";
      return;
    }
    // TODO support multiple POIs (multiple bugs for the same benchmark version)
    POISpec poi = parse_whole_poi_file(m_whole_poi, m_benchmark);
    if (poi.filename.empty()
        || poi.linum == 0
        || poi.type.empty()
        ) {
      return;
    }
    for (auto it=m_files.begin();it!=m_files.end();it++) {
      std::string filename = *it;
      if (filename.find(poi.filename) != std::string::npos) {
        // Reader reader(filename, poi);
        hebi(filename, poi);
      }
    }
    // process whole poi to match current benchmark
  } else {
    for (auto it=m_files.begin();it!=m_files.end();it++) {
      Reader reader(*it);
      // reader.Read();
    }
  }
  // double t2 = utils::get_time();
  std::cerr << "********** End of Helium **********"  << "\n";
  if (PrintOption::Instance()->Has(POK_BuildRate)) {
    std::cerr << "Compile Success Count: " << g_compile_success_no  << "\n";
    std::cerr << "Compile Error Count: " << g_compile_error_no  << "\n";
    std::cerr << "Buildrate: " << (double)g_compile_success_no / (double)(g_compile_success_no + g_compile_error_no)  << "\n";
  }
  // ExpASTDump::Instance()->time = t2 - t1;

  /**
   * Now its time to dump the *Dump clases
   */
  // std::cout << "====DUMP START========="  << "\n";
  // std::cout << ExpASTDump::Instance()->GetHeader()  << "\n";
  // std::cout << ExpASTDump::Instance()->dump() << "\n";
  // std::cout << "====DUMP STOP========="  << "\n";

  /**
   * Also, store one version to the file.
   */
  // utils::append_file("dump_out.txt", ExpASTDump::Instance()->dump() + "\n");
  // ExpASTDump::Instance()->AppendData();
  // std::cout << BuildRatePlotDump::Instance()->dump()  << "\n";
}
