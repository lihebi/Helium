#ifndef __RESOLVER_HPP__
#define __RESOLVER_HPP__

#include <string>
#include <vector>
#include <set>
#include <regex>
#include "snippet/Snippet.hpp"


class Resolver {
public:
  static std::set<std::string> ExtractToResolve(const std::string& code);
private:
};

#endif
