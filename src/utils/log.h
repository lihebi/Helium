#ifndef LOG_H
#define LOG_H

#include "common.h"

void helium_log(std::string s);
void helium_log_warning(std::string s);
void helium_log_trace(std::string s);

void helium_dump(std::string s);

#endif /* LOG_H */
