#ifndef LOG_H
#define LOG_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <iostream>

void helium_log(std::string s);
void helium_log_warning(std::string s);
void helium_log_trace(std::string s);

void helium_print_trace(std::string s);
void helium_print_warning(std::string s);

void helium_dump(std::string s);

void helium_dump_compile_error(std::string s);

#endif /* LOG_H */
