#include "log.h"
#include "utils.h"
void helium_log(std::string s) {
  utils::append_file("helium_log.txt", s + "\n");
}

/**
 * dump will not dump new line!
 */
void helium_dump(std::string s) {
  utils::append_file("helium_dump.txt", s);
}

void helium_log_warning(std::string s) {
  utils::append_file("helium_log.txt", "WW: " + s + "\n");
}

void helium_log_trace(std::string s) {
  utils::append_file("helium_log.txt", "TT: " + s + "\n");
}
