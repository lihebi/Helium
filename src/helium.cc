#include "helium.h"
#include <string>
#include <iostream>
#include "arg_parser.h"
#include "snippet.h"
#include "resolver.h"
#include "config.h"
#include "utils.h"
#include "reader.h"
#include "helium_utils.h"

#include <gtest/gtest.h>

using namespace utils;

static void
create_tagfile(const std::string& folder, const std::string& file) {
  std::string cmd = "ctags -f ";
  cmd += file;
  cmd += " --languages=c,c++ -n --c-kinds=+x --exclude=heium_result -R ";
  cmd += folder;
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
    if (args.Has("output")) {
      std::string output_file = args.GetString("output");
      if (output_file.empty()) output_file = "tags";
      create_tagfile(m_folder, output_file);
    }
    exit(0);
  }

  if (args.Has("create-srcml")) {
    ast::Doc doc;
    utils::file2xml(m_folder, doc);
    if (args.Has("output")) {
      std::string output_file = args.GetString("output");
      if (output_file.empty()) {
        // FIXME not working
        doc.print(std::cout);
      } else {
        doc.save_file(output_file.c_str());
      }
    }
    exit(0);
  }



  /*******************************
   ** Helium start
   *******************************/

  
  /* load tag file */
  std::string tagfile = args.GetString("tagfile");
  if (tagfile.empty()) {
    // ctags_load(m_folder + "/tags");
    // create tagfile
    std::cout << "creating tag file ..."  << "\n";
    create_tagfile(m_folder, "/tmp/helium.tags");
    std::cout << "done"  << "\n";
    ctags_load("/tmp/helium.tags");
  } else {
    ctags_load(tagfile);
  }

  /* load system tag file */
  SystemResolver::Instance()->Load(helium_home + "/systype.tags");
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
    for (auto it=m_files.begin();it!=m_files.end();it++) {
      Reader reader(*it);
      std::cout << "Segment count: " << reader.GetSegmentCount() << "\n";
      reader.PrintSegments();
    }
    exit(0);
  }
  if (args.Has("print-segment-info")) {
    for (auto it=m_files.begin();it!=m_files.end();it++) {
      Reader reader(*it);
      std::cout << "Segment count: " << reader.GetSegmentCount() << "\n";
      std::cout << "segment size: " << reader.GetSegmentLOC() << "\n";
    }
    exit(0);
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

void
Helium::Run() {
  /**
   * Code selection method is given as a file, so this is a config file, contains the <file:line> format
   */
  if (utils::file_exists(Config::Instance()->GetString("code-selection"))) {
    // code selection method is a config file, we only need to check those files in the config.
    std::map<std::string, std::vector<int> > conf = parse_selection_conf(Config::Instance()->GetString("code-selection"));
    for (auto c : conf) {
      std::string filename = m_folder + "/" + c.first;
      if (utils::file_exists(filename)) {
        std::cout << "processing: " << filename << " ...\n";
        Reader reader(filename, c.second);
        reader.PrintSegments();
        reader.Read();
      }
    }
  } else {
    for (auto it=m_files.begin();it!=m_files.end();it++) {
      Reader reader(*it);
      reader.Read();
    }
  }
}


