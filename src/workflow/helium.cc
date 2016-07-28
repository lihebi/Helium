#include "helium.h"
#include <string>
#include <iostream>


#include "reader.h"
#include "helium_utils.h"

#include "config/arg_parser.h"
#include "config/config.h"
#include "config/options.h"

#include "parser/ast.h"
#include "parser/ast_node.h"

#include "utils/utils.h"
#include "utils/dump.h"

#include "resolver/snippet.h"
#include "resolver/resolver.h"
#include "resolver/snippet_db.h"
#include "resolver/header_dep.h"

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
  ArgParser args(argc, argv);



  /* load config */
  Config::Instance()->ParseFile(helium_home+"/helium.conf");
  Config::Instance()->Overwrite(args);

  if (args.Has("print-config")) {
    std::cout << Config::Instance()->ToString() << "\n";
    exit(0);
  }

  if (args.Has("check-headers")) {
    SystemResolver::check_headers();
    exit(0);
  }

  /*******************************
   ** BEGIN need folder argument
   *******************************/
  
  // target folder
  m_folder = args.GetString("folder");
  if (m_folder.empty()) {
    args.PrintHelp();
    exit(1);
  }
  if (utils::is_dir(m_folder)) {
    while (m_folder.back() == '/') m_folder.pop_back();
  } else if (utils::is_file(m_folder)) {
  } else {
    std::cerr<<"no such folder" << m_folder<<'\n';
    assert(false);
  }




  /*******************************
   ** utilities
   *******************************/
  if (args.Has("create-tagfile")) {
    std::string output_file = args.GetString("output");
    if (output_file.empty()) output_file = "tags";
    create_tagfile(m_folder, output_file);
    exit(0);
  }

  if (args.Has("create-snippet-db")) {
    std::string output_folder;
    if (args.Has("output")) {
      output_folder = args.GetString("output");
    }
    if (output_folder.empty()) output_folder = "snippets";
    std::string tagfile;
    if (args.Has("tagfile")) {
      tagfile = args.GetString("tagfile");
    }
    std::string tmpdir = utils::create_tmp_dir();
    create_tagfile(m_folder, tmpdir+"/tags");
    tagfile = tmpdir+"/tags";
    std::cout << "created tagfile: " << tagfile  << "\n";
    SnippetDB::Instance()->Create(tagfile, output_folder);
    exit(0);
  }

  // if (args.Has("create-srcml")) {
  //   ast::Doc doc;
  //   utils::file2xml(m_folder, doc);
  //   if (args.Has("output")) {
  //     std::string output_file = args.GetString("output");
  //     if (output_file.empty()) {
  //       // FIXME not working
  //       doc.print(std::cout);
  //     } else {
  //       doc.save_file(output_file.c_str());
  //     }
  //   }
  //   exit(0);
  // }

  if (args.Has("print-header-deps")) {
    HeaderSorter::Instance()->Load(m_folder);
    HeaderSorter::Instance()->Dump();
    exit(0);
  }

  if (args.Has("create-function-ast")) {
    // this only works for a single file
    if (utils::file_exists(m_folder)) {
      ast::XMLDoc doc;
      utils::file2xml(m_folder, doc);
      ast::NodeList func_nodes = ast::find_nodes(doc, ast::NK_Function);
      for (ast::Node func : func_nodes) {
        ast::AST ast(func);
        std::set<ast::ASTNode*> leafs = ast.GetLeafNodes();
        ast.VisualizeN(leafs, {});
        // std::cout << ast.GetCode() <<"\n";
      }
      exit(0);
    } else {
      std::cerr << "create-function-ast only works for a single file.\n";
      exit(1);
    }
  }



  /*******************************
   ** Helium start
   *******************************/

  
  /* load tag file */
  std::string tagfile = args.GetString("tagfile");
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

  std::string snippet_db_folder = args.GetString("snippet-db-folder");
  if (snippet_db_folder.empty()) {
    std::cerr << "snippet database folder unset. Please use --snippet-db-folder option. --help for more details." << "\n";
    assert(false);
  }
  SnippetDB::Instance()->Load(snippet_db_folder);

  if (args.Has("create-header-dep")) {
    HeaderDep::Create(m_folder);
    exit(0);
  }
  HeaderDep::Instance()->LoadFromDB();
  if (args.Has("print-header-dep")) {
    HeaderDep::Instance()->Dump();
    exit(0);
  }

  if (args.Has("print-callgraph")) {
    SnippetDB::Instance()->PrintCG();
    exit(0);
  }

  /* load system tag file */
  SystemResolver::Instance()->Load(helium_home + "/systype.tags");
  // DEPRECATED
  HeaderSorter::Instance()->Load(m_folder);

  
  // std::string output_folder = Config::Instance()->GetString("output-folder");
  // assert(!output_folder.empty() && "output-folder is not set");
  // /* prepare folder */
  // remove_folder(output_folder);
  // create_folder(output_folder);

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

  if (args.Has("print-segments")) {
    assert(false);
    for (auto it=m_files.begin();it!=m_files.end();it++) {
      Reader reader(*it);
      // std::cout << "Segment count: " << reader.GetSegmentCount() << "\n";
      reader.PrintSegments();
    }
    exit(0);
  }
  if (args.Has("print-segment-info")) {
    assert(false);
    for (auto it=m_files.begin();it!=m_files.end();it++) {
      Reader reader(*it);
      // std::cout << "Segment count: " << reader.GetSegmentCount() << "\n";
      // std::cout << "segment size: " << reader.GetSegmentLOC() << "\n";
    }
    exit(0);
  }

  if (args.Has("slice")) {
    // use slicing as code selection method
    std::string slice_file = args.GetString("slice");
    Reader::slice(slice_file, m_folder);
    std::cout << BuildRatePlotDump::Instance()->dump()  << "\n";
    exit(0);
  }

  /**
   * TODO so many singletons are good practice? Maybe I need to make the Helium class singleton and accessible, and make these singletons its fields.
   */
  if (args.Has("slice-file")) {
    std::string slice_file = args.GetString("slice-file");
    SimpleSlice::Instance()->SetSliceFile(slice_file);
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
    ast::XMLDoc doc;
    utils::file2xml(file, doc);
    ast::XMLNodeList nodes = ast::find_nodes(doc, ast::NK_Function);
    ret += nodes.size();
  }
  return ret;
}

void
Helium::Run() {
  /**
   * Print some meta data for the benchmark project.
   */
  std::cerr << "***** Helium Benchmark Meta *****"  << "\n";
  std::cerr << "** total file: " << m_files.size()  << "\n";
  int func_count = countFunction();
  std::cerr << "** totla function: " << func_count  << "\n";
  std::cerr << "*********************************" << "\n";
  ExpASTDump::Instance()->file_count = m_files.size();
  ExpASTDump::Instance()->func_count = func_count;
  ExpASTDump::Instance()->benchmark = m_folder;
  // double t1 = utils::get_time();
  /**
   * Code selection method is given as a file, so this is a config file, contains the <file:line> format
   */
  if (utils::file_exists(Config::Instance()->GetString("code-selection"))) {
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


