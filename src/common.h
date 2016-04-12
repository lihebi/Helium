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

#include <boost/regex.hpp>

extern const std::set<std::string> c_common_keywords;
extern const std::set<std::string> c_extend_keywords;
extern int global_seg_no;
extern int g_compile_success_no;
extern int g_compile_error_no;

bool
is_c_keyword(const std::string& s);
std::set<std::string>
extract_id_to_resolve(const std::string& code);

#endif
