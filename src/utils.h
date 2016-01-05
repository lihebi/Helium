#ifndef __UTIL_H__
#define __UTIL_H__

#include "common.h"
#include <pugixml.hpp>

namespace utils {

  // inline std::string& ltrim(std::string &s);
  // inline std::string& rtrim(std::string &s);
  // inline std::string& trim(std::string &s);
  
  /**
   * Trim a string. Modify in position
   */
  // trim from start
  inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
  }

  // trim from end
  inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
  }

  // trim from both ends
  inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
  }


  /*******************************
   ** File utils
   *******************************/

  std::vector<std::string> get_files(const std::string &folder);
  void get_files(const std::string &folder, std::vector<std::string>& vs);
  void get_files_by_extension(const std::string& folder, std::vector<std::string>& vs, const std::string& s);
  void get_files_by_extension(const std::string& folder, std::vector<std::string>& vs, const std::vector<std::string>& extension);
  bool file_exist(const std::string& file);


  /*******************************
   * read/write
   *******************************/
  void write_file(const std::string& file, const std::string& content);
  void append_file(const std::string& file, const std::string& content);
  std::string read_file(const std::string& file);
  // folders
  void remove_folder(const std::string& folder);
  void create_folder(const std::string& folder);

  /*******************************
   ** Srcml Utils
   *******************************/

  void file2xml(const std::string& filename, pugi::xml_document& doc);
  void string2xml(const std::string& code, pugi::xml_document& doc);

  /*******************************
   ** string
   *******************************/

  bool ends_with(const std::string &s, const std::string &pattern);
  bool starts_with(const std::string &s, const std::string &pattern);
  std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems);
  std::vector<std::string> split(const std::string &s, char delim);

  /*
   * This is the split function that use any white space as delimiter
   * Use completely different implementation from the other two
   */
  std::vector<std::string> split(const std::string &s);
  void remove(std::string& s, const std::string& pattern);

  /*******************************
   ** thread
   *******************************/
  std::string exec(const char* cmd, int *status, int timeout=0);
  // with input
  std::string exec(const char* cmd, const char* input, int *status, unsigned int timeout=0);

}

#endif
