#ifndef __STRING_UTIL_HPP__
#define __STRING_UTIL_HPP__

#include <string>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

class StringUtil {
public:
  static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
  }
  static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
  }
  static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
  }

  static bool EndsWith(const std::string &s, const std::string &pattern);
  static bool StartsWith(const std::string &s, const std::string &pattern);
};

#endif
