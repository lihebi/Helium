#ifndef __UTIL_H__
#define __UTIL_H__

#include "common.h"

#include <pugixml.hpp>
#include <stdlib.h>
#include <sys/types.h>

#include "rand_utils.h"

#include "xml_utils.h"
#include "string_utils.h"
#include "thread_utils.h"
#include "fs_utils.h"
#include "color_utils.h"

namespace utils {
  bool is_number(const std::string& s);

  int get_line_number(std::string filename, std::string pattern);
  std::vector<int> get_line_numbers(std::string filename, std::string pattern);

  double get_time();
  void debug_time(std::string id="");

}

#endif
