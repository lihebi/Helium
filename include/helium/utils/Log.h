#ifndef LOG_H
#define LOG_H

#include "Common.h"

void helium_log(std::string s);
void helium_log_warning(std::string s);
void helium_log_trace(std::string s);

void helium_print_trace(std::string s);
void helium_print_warning(std::string s);

void helium_dump(std::string s);

void helium_dump_compile_error(std::string s);

#endif /* LOG_H */
