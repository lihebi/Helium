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
trim(std::string& s) {
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
split(const std::string &s) {
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
split(const std::string &s, char delim, std::vector<std::string> &elems) {
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
split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}


/**
 * If string s ends with string pattern.
 */
bool ends_with(const std::string &s, const std::string &pattern) {
  if (s.length() >= pattern.length()) {
    return (0 == s.compare (s.length() - pattern.length(), pattern.length(), pattern));
  } else {
    return false;
  }
}
/**
 * If string s starts with string pattern.
 */
bool starts_with(const std::string &s, const std::string &pattern) {
  if (s.length() >= pattern.length()) {
    return (0 == s.compare (0, pattern.length(), pattern));
  } else {
    return false;
  }
}

/**
 * remove all currence of pattern from s
 */
void remove(std::string& s, const std::string& pattern) {
  while (s.find(pattern) != std::string::npos) {
    s.erase(s.find(pattern), pattern.length());
  }
}




// gtest document says the test_case_name and test_name should not contain _
TEST(trim_test_case, trim_test) {
  std::string s = " ab cd  ";
  trim(s);
  EXPECT_EQ(s, "ab cd");
  s = "\tabcd\t   \n";
  trim(s);
  EXPECT_EQ(s, "abcd");
}


/*******************************
 ** DomUtil
 *******************************/

/**
 * valid ast includes: expr, decl, break, macro, for, while, if, function
 */
bool is_valid_ast(const char* name) {
  if (strcmp(name, "expr_stmt") == 0
    || strcmp(name, "decl_stmt") == 0
    || strcmp(name, "break") == 0
    || strcmp(name, "macro") == 0
    || strcmp(name, "for") == 0
    || strcmp(name, "while") == 0
    || strcmp(name, "if") == 0
    || strcmp(name, "function") == 0
  ) return true;
  else return false;
}

pugi::xml_node get_previous_ast_element(pugi::xml_node node) {
  while (node) {
    node = node.previous_sibling();
    if (node) {
      if (is_valid_ast(node.name())) {
        return node;
      }
    }
  }
  // This should be node_null
  return node;
}

pugi::xml_node get_parent_ast_element(pugi::xml_node node) {
  while (node) {
    node = node.parent();
    if (node) {
      if (is_valid_ast(node.name())) {
        return node;
      }
    }
  }
  return node;
}

/**
 * Get the call place of the function in node.
 * @param[in] node the <function> node in xml
 * @return the node <call> of the function. Or null_node is not found.
 */
pugi::xml_node get_function_call(pugi::xml_node node) {
  const char *func_name = node.child_value("name");
  pugi::xpath_node_set call_nodes = node.root().select_nodes("//call");
  for (auto it=call_nodes.begin();it!=call_nodes.end();it++) {
    if (strcmp(it->node().child_value("name"), func_name) == 0) {
      return get_parent_ast_element(it->node());
    }
  }
  pugi::xml_node null_node;
  return null_node;
}

/**
 * Get text content of node.
 * Will traverse xml structure.
 * But will avoid a tag with "helium-omit" ATTR.
 * Add some structure to make the syntax correct. FIXME
 */
std::string
get_text_content(pugi::xml_node node) {
  std::string text;
  if (!node) return "";
  for (pugi::xml_node n : node.children()) {
    if (n.type() == pugi::node_element) {
      if (!node.attribute("helium-omit")) {
        // add text only if it is not in helium-omit
        text += get_text_content(n);
      } else if (strcmp(node.name(), "else") == 0 ||
        strcmp(node.name(), "then") == 0 ||
        strcmp(node.name(), "elseif") == 0 ||
        strcmp(node.name(), "default") == 0
      ) {
        // FIXME Why??????
        // For simplification of code.
        // I will add "helium-omit" attribute on the AST to mark deletion.
        // Those tag will be deleted.
        // But to make the syntax valid, I need to add some "{}"
        text += "{}";
      } else if (strcmp(node.name(), "case") == 0) {
        // FIXME why??????
        text += "{break;}";
      }
    } else {
      text += n.value();
    }
  }
  return text;
}

/**
 * Get text content of node, except <name> tag
 */
std::string
get_text_content_except(pugi::xml_node node, std::string name) {
  if (!node) return "";
  std::string text;
  for (pugi::xml_node n : node.children()) {
    if (n.type() == pugi::node_element) {
      if (!node.attribute("helium-omit")) {
        if (strcmp(n.name(), name.c_str()) != 0) {
          text += get_text_content_except(n, name);
        }
      }
      // TODO this version does not use the trick for simplification,
      // so it doesnot work with simplification
    } else {
      text += n.value();
    }
  }
  return text;
}


// struct simple_walker: pugi::xml_tree_walker {
//   std::string text;
//   virtual bool for_each(pugi::xml_node& node) {
//     if (!node.attribute("helium-omit")) {
//       text += node.value();
//     }
//     return true; // continue traversal
//   }
// };

// std::string GetTextContent(pugi::xml_node node) {
//   simple_walker walker;
//   node.traverse(walker);
//   return walker.text;
// }


/**
 * test if node is within <level> levels inside a <tagname>
 */
bool
in_node(pugi::xml_node node, std::string tagname, int level) {
  while (node.parent() && level>0) {
    node = node.parent();
    level--;
    if (node.type() != pugi::node_element) return false;
    if (node.name() == tagname) return true;
  }
  return false;
}

/**
 * least upper bound of two nodes
 */
pugi::xml_node
lub(pugi::xml_node n1, pugi::xml_node n2) {
  if (n1.root() != n2.root()) return pugi::xml_node();
  pugi::xml_node root = n1.root();
  int num1=0, num2=0;
  pugi::xml_node n;
  n = n1;
  while (n!=root) {
    n = n.parent();
    num1++;
  }
  n = n2;
  while(n!=root) {
    n = n.parent();
    num2++;
  }
  if (num1 > num2) {
    // list 1 is longer
    while(num1-- != num2) {
      n1 = n1.parent();
    }
  } else {
    while(num2-- != num1) {
      n2 = n2.parent();
    }
  }
  // will end because the root is the same
  while (n1 != n2) {
    n1 = n1.parent();
    n2 = n2.parent();
  }
  return n1;
}

/*******************************
 ** FileUtil
 *******************************/

/**
 * Get all files in folder.
 */
std::vector<std::string> get_files(const std::string& folder) {
  std::vector<std::string> vs;
  get_files(folder, vs);
  return vs;
}
/**
 * get all files of the folder.
 * @param[in] folder
 * @param[out] vs
 */
void get_files(const std::string& folder, std::vector<std::string>& vs) {
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
void get_files_by_extension(
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
void get_files_by_extension(
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
write(const std::string& file, const std::string& content) {
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
append(const std::string& file, const std::string& content) {
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
read(const std::string& file) {
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
remove_folder(const std::string& folder) {
  fs::path folder_path(folder);
  if (fs::exists(folder_path)) {
    fs::remove_all(folder_path);
  }
}

/**
 * mkdir -p <folder>
 */
void
create_folder(const std::string& folder) {
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
void file2xml(const std::string &filename, pugi::xml_document& doc) {
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
void string2xml(const std::string &code, pugi::xml_document& doc) {
  std::string cmd = "src2srcml --position -lC";
  // std::string cmd = "src2srcml -lC";
  std::string xml = exec(cmd.c_str(), code.c_str(), NULL);
  doc.load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
}

