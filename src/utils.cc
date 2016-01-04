#include "utils.h"
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <fstream>

namespace fs = boost::filesystem;

/*******************************
 ** string utils
 *******************************/

/**
 * Trim a string. Modify in position
 */
void
utils::trim(std::string& s) {
  size_t begin = s.find_first_not_of(" \n\r\t");
  size_t end = s.find_last_not_of(" \n\r\t")+1;
  s = s.substr(begin, end);
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




// gtest document says the test_case_name and test_name should not contain _
TEST(trim_test_case, trim_test) {
  std::string s = " ab cd  ";
  utils::trim(s);
  EXPECT_EQ(s, "ab cd");
  s = "\tabcd\t   \n";
  utils::trim(s);
  EXPECT_EQ(s, "abcd");
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


/**
 * Write file with content.
 * Will overwrite. If the file doesn't exist, it will create it recursively(the parent directory)
 */
void
utils::write(const std::string& file, const std::string& content) {
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
utils::append(const std::string& file, const std::string& content) {
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
utils::read(const std::string& file) {
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
  fs::path folder_path(folder);
  if (!fs::exists(folder_path)) {
    fs::create_directories(folder_path);
  }
}

/*******************************
 ** SrcmlUtil
 *******************************/

/**
 * convert a file into srcml output
 * @param[in] filename
 * @param[out] doc doc must be created by caller
 */
void utils::file2xml(const std::string &filename, pugi::xml_document& doc) {
  std::string cmd;
  cmd = "src2srcml --position " + filename;
  // cmd = "src2srcml " + filename;
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
  std::string cmd = "src2srcml --position -lC";
  // std::string cmd = "src2srcml -lC";
  std::string xml = exec(cmd.c_str(), code.c_str(), NULL);
  doc.load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
}

