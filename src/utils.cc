#include "utils.h"
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <fstream>

namespace fs = boost::filesystem;

namespace utils {
  const char* RED = "\033[31m";
  const char* GREEN = "\033[1;32m";
  const char* YELLOW = "\033[1;33m";
  const char* BLUE = "\033[1;34m";
  const char* PURPLE = "\033[1;35m";
  const char* CYAN = "\033[1;36m";
  const char* RESET = "\033[0m";

  void print(const char*s, ColorKind k) {
    const char *c;
    switch (k) {
    case CK_Red: c = RED; break;
    case CK_Green: c = GREEN; break;
    case CK_Yellow: c = YELLOW; break;
    case CK_Blue: c = BLUE; break;
    case CK_Purple: c = PURPLE; break;
    case CK_Cyan: c = CYAN; break;
    default:
      return;
    }
    printf("%s%s%s\n", c, s, RESET);
  }

  void print(const std::string &s, ColorKind k) {
    print(s.c_str(), k);
  }
  void print(int i, ColorKind k) {
    print(std::to_string(i), k);
  }


} // end namespace utils


/*******************************
 ** string utils
 *******************************/


// gtest document says the test_case_name and test_name should not contain _
TEST(utils_test_case, trim_test) {
  std::string s = " ab cd  ";
  utils::trim(s);
  EXPECT_EQ(s, "ab cd");
  s = "\tabcd\t   \n";
  utils::trim(s);
  EXPECT_EQ(s, "abcd");
  s = "   \t\n";
  utils::trim(s);
  EXPECT_TRUE(s.empty());
}



/**
 * spilt string based, delimiter is *space*
 * @param s [in] string to be spliteed. Not modified.
 * @return a vector of string
 */
std::vector<std::string>
utils::split(const std::string &s) {
  std::istringstream iss(s);
  std::vector<std::string> tokens{
    std::istream_iterator<std::string>{iss},
    std::istream_iterator<std::string>{}
  };
  return tokens;
}

TEST(utils_test_case, split_test) {
  std::string s = "  ab  cd \n \t ef";
  std::vector<std::string> vs = utils::split(s);
  EXPECT_FALSE(vs.empty());
  EXPECT_EQ(vs.size(), 3);
  EXPECT_EQ(vs[0], "ab");
  EXPECT_EQ(vs[1], "cd");
  EXPECT_EQ(vs[2], "ef");
  vs = utils::split("hello= ", '=');
  ASSERT_EQ(vs.size(), 2);
  vs = utils::split("hello=", '=');
  ASSERT_EQ(vs.size(), 1);
}

/**
 * split string based on delimeter given
 * @param[in] s string to be split
 * @param[in] delim delimeter
 * @param[out] elems store the splits
 * @return reference to elems
 */
std::vector<std::string>&
utils::split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}


/**
 * Create and return a new vector of splits
 */
std::vector<std::string>
utils::split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}


/**
 * If string s ends with string pattern.
 */
bool utils::ends_with(const std::string &s, const std::string &pattern) {
  if (s.length() >= pattern.length()) {
    return (0 == s.compare (s.length() - pattern.length(), pattern.length(), pattern));
  } else {
    return false;
  }
}
/**
 * If string s starts with string pattern.
 */
bool utils::starts_with(const std::string &s, const std::string &pattern) {
  if (s.length() >= pattern.length()) {
    return (0 == s.compare (0, pattern.length(), pattern));
  } else {
    return false;
  }
}

/**
 * remove all currence of pattern from s
 */
void utils::remove(std::string& s, const std::string& pattern) {
  while (s.find(pattern) != std::string::npos) {
    s.erase(s.find(pattern), pattern.length());
  }
}

bool utils::is_number(const std::string& s) {
  std::string::const_iterator it = s.begin();
  while (it != s.end() && std::isdigit(*it)) ++it;
  return !s.empty() && it == s.end();
}

TEST(utils_test_case, is_number_test) {
  EXPECT_TRUE(utils::is_number("442"));
  EXPECT_TRUE(utils::is_number("10"));
  EXPECT_FALSE(utils::is_number("5-2"));
  EXPECT_FALSE(utils::is_number("SIZE"));
}






/*******************************
 ** FileUtil
 *******************************/

/**
 * Get all files in folder.
 */
std::vector<std::string> utils::get_files(const std::string& folder) {
  std::vector<std::string> vs;
  get_files(folder, vs);
  return vs;
}
/**
 * get all files of the folder.
 * @param[in] folder
 * @param[out] vs
 */
void utils::get_files(const std::string& folder, std::vector<std::string>& vs) {
  fs::path project_folder(folder);
  fs::recursive_directory_iterator it(folder), eod;
  BOOST_FOREACH (fs::path const & p, std::make_pair(it, eod)) {
    if (is_regular_file(p)) {
      vs.push_back(p.string());
    }
  }
}

/**
 * Get files in a folder, with ONE specific extension.
 * @param[in] folder
 * @param[out] vs
 * @param[in] extension a suffix as string
 */
void utils::get_files_by_extension(
  const std::string& folder,
  std::vector<std::string>& vs,
  const std::string& extension
) {
  if (!is_dir(folder)) return;
  std::vector<std::string> ve {extension};
  get_files_by_extension(folder, vs, ve);
}

/**
 * Get files in a folder, with some specific extensions.
 * @param[in] folder folder to search
 * @param[out] vs a list of matched files
 * @param[in] extension a list of extensions
 */
void utils::get_files_by_extension(
  const std::string& folder,
  std::vector<std::string>& vs,
  const std::vector<std::string>& extension
) {
  if (!is_dir(folder)) return;
  fs::path project_folder(folder);
  fs::recursive_directory_iterator it(folder), eod;
  BOOST_FOREACH(fs::path const& p, std::make_pair(it, eod)) {
    if (is_regular_file(p)) {
      std::string with_dot = p.extension().string();
      // the extension may not exist
      if (!with_dot.empty()) {
        // if extension does not exist,
        // the substr(1) will cause exception of out of range of basic_string
        std::string without_dot = with_dot.substr(1);
        if (std::find(extension.begin(), extension.end(), with_dot) != extension.end()
        || std::find(extension.begin(), extension.end(), without_dot) != extension.end()) {
          vs.push_back(p.string());
        }
      }
    }
  }
}

bool
utils::file_exists(const std::string& file) {
  fs::path file_path(file);
  return fs::exists(file_path);
}

bool utils::is_file(const std::string &file) {
  struct stat sb;
  if (stat(file.c_str(), &sb) == 0 && S_ISREG(sb.st_mode)) {
    return true;
  }
  return false;
}
bool utils::is_dir(const std::string &file) {
  struct stat sb;
  if (stat(file.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
    return true;
  }
  return false;
}
TEST(utils_test_case, is_file_dir) {
  EXPECT_TRUE(utils::is_file("/usr/include/stdio.h"));
  EXPECT_FALSE(utils::is_file("/usr/include"));
  EXPECT_FALSE(utils::is_file("/fjlsdjn/fdj"));

  EXPECT_FALSE(utils::is_dir("/usr/include/stdio.h"));
  EXPECT_TRUE(utils::is_dir("/usr/include"));
  EXPECT_FALSE(utils::is_dir("/fjlsdjn/fdj"));

}

/**
 * Write file with content.
 * Will overwrite. If the file doesn't exist, it will create it recursively(the parent directory)
 */
void
utils::write_file(const std::string& file, const std::string& content) {
  fs::path file_path(file);
  fs::path dir = file_path.parent_path();
  if (!fs::exists(dir)) {
    fs::create_directories(dir);
  }
  std::ofstream os;
  os.open(file_path.string());
  if (os.is_open()) {
    os << content;
    os.close();
  }
}

/**
 * Append content to file. Create if not existing.
 */
void
utils::append_file(const std::string& file, const std::string& content) {
  fs::path file_path(file);
  fs::path dir = file_path.parent_path();
  if (!fs::exists(dir)) {
    fs::create_directories(dir);
  }
  std::ofstream os;
  os.open(file_path.string(), std::ios_base::app);
  if (os.is_open()) {
    os<<content;
    os.close();
  }
}

/**
 * Read a file into string.
 */
std::string
utils::read_file(const std::string& file) {
  std::ifstream is;
  is.open(file);
  std::string code;
  if (is.is_open()) {
    std::string line;
    while(getline(is, line)) {
      code += line+"\n";
    }
    is.close();
  }
  return code;
}

/**
 * rm -r <folder>
 */
void
utils::remove_folder(const std::string& folder) {
  fs::path folder_path(folder);
  if (fs::exists(folder_path)) {
    fs::remove_all(folder_path);
  }
}

/**
 * mkdir -p <folder>
 */
void
utils::create_folder(const std::string& folder) {
  if (folder.empty()) return;
  fs::path folder_path(folder);
  if (!fs::exists(folder_path)) {
    fs::create_directories(folder_path);
  }
}

/**
 * create tmp dir, return it.
 * @input s /tmp/helium-XXXXXX (must have 6 X at the end.)
 */
std::string utils::create_tmp_dir(std::string s) {
  // char tmp_dir[] = "/tmp/helium-test-temp.XXXXXX";
  std::string sub = s.substr(s.find_last_not_of('X'));
  if (sub.size() !=7) return "";
  sub = sub.substr(1);
  assert(sub.size() == 6 && "tmp dir url format error!");
  if (sub.find_first_not_of('X') != std::string::npos) return "";
  char tmp_dir[s.size()+1];
  strcpy(tmp_dir, s.c_str());
  char *result = mkdtemp(tmp_dir);
  if (result == NULL) return "";
  std::string dir = tmp_dir;
  return dir;
}

TEST(utils_test_case, tmp_dir) {
  std::string s = utils::create_tmp_dir("/garjb");
  EXPECT_EQ(s, "");
  s = utils::create_tmp_dir("/tmp/helium.XXXXXX");
  EXPECT_NE(s, "");
  EXPECT_TRUE(utils::is_dir(s));
}

/*******************************
 ** SrcmlUtil
 *******************************/

/**
 * Query query on code.
 * return the first matching.
 */
std::string utils::query_xml_first(const std::string& xml_file, const std::string& query) {
  pugi::xml_document doc;
  file2xml(xml_file, doc);
  pugi::xml_node root_node = doc.document_element();
  pugi::xml_node node = root_node.select_node(query.c_str()).node();
  return node.child_value();
}
/**
 * Query "query" on "code".
 * Return all matching.
 * Will not use get_text_content, but use child_value() for a xml tag.
 * Only support tag value currently, not attribute value.
 */
std::vector<std::string> utils::query_xml(const std::string& xml_file, const std::string& query) {
  std::vector<std::string> result;
  pugi::xml_document doc;
  file2xml(xml_file, doc);
  pugi::xml_node root_node = doc.document_element();
  pugi::xpath_node_set nodes = root_node.select_nodes(query.c_str());
  for (auto it=nodes.begin();it!=nodes.end();it++) {
    pugi::xml_node node = it->node();
    result.push_back(node.child_value());
  }
  return result;
}




/**
 * convert a file into srcml output
 * @param[in] filename
 * @param[out] doc doc must be created by caller
 */
void utils::file2xml(const std::string &filename, pugi::xml_document& doc) {
  std::string cmd;
  cmd = "srcml --position " + filename;
  // cmd = "srcml " + filename;
  std::string xml = exec(cmd.c_str(), NULL);
  doc.load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
}
// c code will be converted into xml, and loaded by doc
/**
 * Convert a string of code into srcml output.
 * Only support C code for now.
 * @param[in] code
 * @param[out] doc
 */
void utils::string2xml(const std::string &code, pugi::xml_document& doc) {
  std::string cmd = "srcml --position -lC";
  // std::string cmd = "srcml -lC";
  std::string xml = exec_in(cmd.c_str(), code.c_str(), NULL);
  doc.load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
}


/**
 * Get the line numbers in file "filename", that matches the pattern.
 */
std::vector<int> utils::get_line_numbers(std::string filename, std::string pattern) {
  std::ifstream is;
  is.open(filename);
  int line_number = 0;
  std::vector<int> result;
  if (is.is_open()) {
    std::string line;
    while(getline(is, line)) {
      line_number++;
      if (line.find(pattern) != std::string::npos) {
        result.push_back(line_number);
      }
    }
    is.close();
  }
  return result;
}


/**
 * Get only the first number line matching the pattern
 */
int utils::get_line_number(std::string filename, std::string pattern) {
  std::ifstream is;
  is.open(filename);
  int line_number = 0;
  if (is.is_open()) {
    std::string line;
    while(getline(is, line)) {
      line_number++;
      if (line.find(pattern) != std::string::npos) {
        is.close();
        return line_number;
      }
    }
    is.close();
  }
  return 0;
}

TEST(utils_test_case, get_line_number) {
const char *raw = R"prefix(
2
hello world
really this /* @HeliumLineMark */
this lien contains none
6

8 the previous is empyt line
@HeliumLineMark

)prefix";
 std::string dir = utils::create_tmp_dir("/tmp/helium-test.XXXXXX");
 utils::write_file(dir+"/a.txt", raw);
 // std::cout <<dir+"/a.txt"  << "\n";
 int line = utils::get_line_number(dir+"/a.txt", "@HeliumLineMark");
 EXPECT_EQ(line, 4);
 std::vector<int> lines = utils::get_line_numbers(dir+"/a.txt", "@HeliumLineMark");
 ASSERT_EQ(lines.size(), 2);
 EXPECT_EQ(lines[0], 4);
 EXPECT_EQ(lines[1], 9);
}


void utils::seed_rand() {
  srand(time(0));
}

int utils::rand_int(int low, int high) {
  assert(high >= low);
  int a = rand();
  return a % (high - low + 1) + low;
}
