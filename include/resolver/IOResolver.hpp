#ifndef __IO_RESOLVER_HPP__
#define __IO_RESOLVER_HPP__

#include <pugixml/pugixml.hpp>
#include <vector>
#include <set>

#include "variable/Variable.hpp"
#include "segment/Segment.hpp"

class IOResolver {
public:
  IOResolver();
  ~IOResolver();
  static std::shared_ptr<Variable> ResolveLocalVar(const std::string& var_name, pugi::xml_node node);
  // recursively resolve
  // output: resolved
  static void ResolveAliveVars(pugi::xml_node node, std::set<std::shared_ptr<Variable> >& resolved);
  // get undefined variables and resolve the type
  static std::set<std::shared_ptr<Variable> > ResolveUndefinedVars(
    const Segment& segment
  );
  static std::set<std::shared_ptr<Variable> > ResolveUndefinedVars(
    pugi::xml_node node
  );
  // get the variable name only
  void GetUndefinedVars(const Segment& segment, std::vector<std::string>& resolved);
  void GetUndefiendVars(pugi::xml_node node, std::vector<std::string>& resolved);
private:
  static void resolveUndefinedVars(
    std::vector<pugi::xml_node> vn,
    std::set<std::shared_ptr<Variable> >& resolved,
    std::set<std::shared_ptr<Variable> > defined
  );
  static void visit(
    pugi::xml_node node,
    std::set<std::shared_ptr<Variable> >& resolved,
    std::set<std::shared_ptr<Variable> >& defined
  );
};

#endif
