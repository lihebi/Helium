#ifndef __FILE_UTIL_HPP__
#define __FILE_UTIL_HPP__

#include <vector>
#include <string>

class FileUtil {
public:
  // get file of the folder
  static std::vector<std::string> GetFiles(const std::string &folder);
  // get file of the folder, store in vs
  static void GetFiles(const std::string &folder, std::vector<std::string>& vs);
  // by single extension
  static void GetFilesByExtension(
    const std::string& folder,
    std::vector<std::string>& vs,
    const std::string& s
  );
  // by multiple extension
  static void GetFilesByExtension(
    const std::string& folder, // folder
    std::vector<std::string>& vs, // output
    const std::vector<std::string>& extension // extension
  );
  static std::string GetBlock(const std::string& filename, int line, char type);

  // IO helper
  static void Write(
    const std::string& file,
    const std::string& content
  );
  static void
  Append(
         const std::string& file,
         const std::string& content
         );

  static std::string Read(const std::string& file);

  // folders
  static void RemoveFolder(const std::string& folder);
  static void CreateFolder(const std::string& folder);
};

#endif
