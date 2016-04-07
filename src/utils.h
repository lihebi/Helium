#ifndef __UTIL_H__
#define __UTIL_H__

#include "common.h"
#include <pugixml.hpp>
#include <stdlib.h>

namespace utils {

  extern const char* RED;
  extern const char* GREEN;
  extern const char* YELLOW;
  extern const char* BLUE;
  extern const char* PURPLE;
  extern const char* CYAN;
  extern const char* RESET;

  typedef enum {
    CK_Red,
    CK_Green,
    CK_Yellow,
    CK_Blue,
    CK_Purple,
    CK_Cyan,
    CK_Reset
  } ColorKind;

  void print(const char*s, ColorKind k);

  void print(const std::string &s, ColorKind k);
  void print(int i, ColorKind k);

  // inline void red(const std::string&s)   {print(s, CK_Red);}
  // inline void red(const char*s)          {print(s, CK_Red);}
  // inline void red(int i)                 {print(i, CK_Red);}
  // inline void green(const std::string&s) {print(s, CK_Green);}
  // inline void green(const char*s)        {print(s, CK_Green);}
  // inline void cyan(const std::string&s)  {print(s, CK_Cyan);}
  // inline void cyan(const char*s)         {print(s, CK_Cyan);}

  // inline void purple(const std::string&s){print(s, CK_Purple);}
  // inline void purple(const char*s)       {print(s, CK_Purple);}
  // inline void blue(const std::string&s)  {print(s, CK_Blue);}
  // inline void blue(const char*s)         {print(s, CK_Blue);}
  // inline void yellow(const std::string&s){print(s, CK_Yellow);}
  // inline void yellow(const char*s)       {print(s, CK_Yellow);}
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
  bool file_exists(const std::string& file);
  bool is_file(const std::string &file);
  bool is_dir(const std::string &file);
  bool exists(const std::string &file);
  std::string full_path(const std::string &file_or_dir);
  /*******************************
   * read/write
   *******************************/
  void write_file(const std::string& file, const std::string& content);
  void append_file(const std::string& file, const std::string& content);
  std::string read_file(const std::string& file);
  // folders
  void remove_folder(const std::string& folder);
  void create_folder(const std::string& folder);
  void remove_file(const std::string &file);

  /**
   * TODO Will generate format according to "filename"
   * If filename do not have a valid extension, use ".dot"
   * Will open the result.
   * ".dot" will be added
   * @param [in] filename The simple filename
   */
  void visualize_dot_graph(const std::string& dot, std::string filename="out");

  std::string create_tmp_dir(std::string s="/tmp/helium-XXXXXX");

  std::vector<std::string> query_xml(const std::string& xml_file, const std::string& query);
  std::string query_xml_first(const std::string& xml_file, const std::string& query);


  /*******************************
   ** Srcml Utils
   *******************************/

  pugi::xml_document* file2xml(const std::string &filename);
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

  bool is_number(const std::string& s);

  /*******************************
   ** thread
   *******************************/
  std::string exec(const char* cmd, int *status=NULL, int timeout=0);
  // with input
  std::string exec_in(const char* cmd, const char* input, int *status=NULL, unsigned int timeout=0);

  int get_line_number(std::string filename, std::string pattern);
  std::vector<int> get_line_numbers(std::string filename, std::string pattern);

  /*******************************
   ** random
   *******************************/
  void seed_rand();
  /**
   * Need to call seed_rand() before this.
   * [low, high], inclusive
   */
  int rand_int(int low, int high);

  int clock_gettime(int /*clk_id*/, struct timespec* t);
  double get_time();
  void debug_time(std::string id="");
}

#endif
