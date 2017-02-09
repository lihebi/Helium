#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <iostream>

#include <boost/regex.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

extern const std::set<std::string> c_common_keywords;
extern const std::set<std::string> c_extend_keywords;
extern int global_seg_no;
extern int g_compile_success_no;
extern int g_compile_error_no;

extern const std::string flush_output;


class HeliumException : public std::exception {
public:
  HeliumException(std::string text) {
    m_text = text;
  }
  virtual const char *what() const throw() {
    return "Helium Exception";
  }
private:
  std::string m_text;
};

bool
is_c_keyword(const std::string& s);

void error(std::string err_msg);

#endif
