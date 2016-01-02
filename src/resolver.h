#ifndef __RESOLVER_H__
#define __RESOLVER_H__

#include <readtags.h>
#include "common.h"
#include "segment.h"
#include "snippet.h"

std::set<std::string>
extract_id_to_resolve(const std::string& code);
bool
is_c_keyword(const std::string& s);



class HeaderSorter {
public:
  static HeaderSorter* Instance() {
    if (m_instance == 0) {
      m_instance = new HeaderSorter();
    }
    return m_instance;
  }
  // load all the header files inside the folder recursively,
  // scan the #inlcude "" statement, and get dependence relations between them
  void Load(const std::string& folder);
  // sort the headers by dependence
  std::vector<std::string> Sort(std::set<std::string> headers);
private:
  bool sortOneRound(std::vector<std::string> &sorted);
  bool isDependOn(const std::string& lhs, const std::string& rhs);
  void addDependence(const std::string& lhs, const std::string& rhs);
  HeaderSorter() {}
  ~HeaderSorter() {}
  static HeaderSorter* m_instance;

  // std::vector<std::string> m_headers;
  std::map<std::string, std::set<std::string> > m_dependence_map;
};

class IOResolver {
public:
  IOResolver();
  ~IOResolver();
  static std::shared_ptr<Variable> ResolveLocalVar(const std::string& var_name, pugi::xml_node node);
  // recursively resolve
  // output: resolved
  static void ResolveAliveVars(
    pugi::xml_node node,
    std::set<std::shared_ptr<Variable> >& resolved,
    const Segment& context
  );
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
