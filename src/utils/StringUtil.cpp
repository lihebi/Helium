#include "util/StringUtil.hpp"
#include <sstream>

std::string StringUtil::SubStringIfFind(const std::string& s, const char c) {
  return s.substr(0, s.find(c));
}

std::vector<std::string>
StringUtil::Split(const std::string &s) {
  std::istringstream iss(s);
  std::vector<std::string> tokens{
    std::istream_iterator<std::string>{iss},
    std::istream_iterator<std::string>{}
  };
  return tokens;
}

std::vector<std::string> &StringUtil::Split(
  const std::string &s,
  char delim,
  std::vector<std::string> &elems
) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}


std::vector<std::string> StringUtil::Split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  Split(s, delim, elems);
  return elems;
}
