#include "fs_utils.h"

namespace utils {
  std::string escape_tide(std::string path) {
    if (path.empty()) return path;
    if (path[0] == '~') {
      const char *home = getenv("HOME");
      if (home) {
        path.replace(0, 1, home);
      }
    }
    return path;
  }
}
