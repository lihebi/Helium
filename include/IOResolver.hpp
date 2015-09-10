#ifndef __IO_RESOLVER_HPP__
#define __IO_RESOLVER_HPP__

#include <pugixml/pugixml.hpp>
#include <vector>

#include "Variable.hpp"
#include "Segment.hpp"

class IOResolver {
public:
  IOResolver();
  ~IOResolver();
  Variable ResolveLocalType(const std::string& var_name, pugi::xml_node node);
  // recursively resolve
  // output: resolved
  void ResolveAliveVars(pugi::xml_node node, std::vector<Variable>& resolved);
  // get undefined variables and resolve the type
  void ResolveUndefinedVars(const Segment& segment, std::vector<Variable>& resolved);
  void ResolveUndefinedVars(pugi::xml_node node, std::vector<Variable>& resolved);
  // get the variable name only
  void GetUndefinedVars(const Segment& segment, std::vector<std::string>& resolved);
  void GetUndefiendVars(pugi::xml_node node, std::vector<std::string>& resolved);
private:
};

#endif
