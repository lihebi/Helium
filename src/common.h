#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>

#include <boost/regex.hpp>

extern const std::set<std::string> c_common_keywords;
extern const std::set<std::string> c_extend_keywords;
extern int global_seg_no;
extern int g_compile_success_no;
extern int g_compile_error_no;

#endif
