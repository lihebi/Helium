#ifndef __IO_RESOLVER_HPP__
#define __IO_RESOLVER_HPP__

#include <pugixml/pugixml.hpp>
#include <vector>
#include <set>

#include "Variable.hpp"
#include "segment/Segment.hpp"

class IOResolver {
public:
  IOResolver();
  ~IOResolver();
  static Variable ResolveLocalVar(const std::string& var_name, pugi::xml_node node);
  // recursively resolve
  // output: resolved
  static void ResolveAliveVars(pugi::xml_node node, std::set<Variable>& resolved);
  // get undefined variables and resolve the type
  static void ResolveUndefinedVars(
    const Segment& segment,
    std::set<Variable>& resolved
  );
  static void ResolveUndefinedVars(
    pugi::xml_node node,
    std::set<Variable>& resolved
  );
  // get the variable name only
  void GetUndefinedVars(const Segment& segment, std::vector<std::string>& resolved);
  void GetUndefiendVars(pugi::xml_node node, std::vector<std::string>& resolved);
private:
  static void resolveUndefinedVars(
    pugi::xml_node node,
    std::set<Variable>& resolved,
    std::set<Variable>& defined
  );
  static void resolveUndefinedVars(
    std::vector<pugi::xml_node> vn,
    std::set<Variable>& resolved,
    std::set<Variable>& defined
  );
  static void visit(
    pugi::xml_node node,
    std::set<Variable>& resolved,
    std::set<Variable>& defined
  );
};

#endif
