#ifndef FS_UTILS_H
#define FS_UTILS_H

#include <string>
#include <vector>

namespace utils {
  std::string escape_tide(std::string path);

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
   * Get the last component of the path.
   * /path/to/a.c => a.c
   * /path/to/ => to
   * path/to/a.c => a.c
   */
  std::string file_name_last_component(std::string filename);
  std::string create_tmp_dir(std::string s="/tmp/helium-XXXXXX");
}

#endif /* FS_UTILS_H */
