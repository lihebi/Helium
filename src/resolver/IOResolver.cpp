#include "resolver/IOResolver.hpp"
#include "util/DomUtil.hpp"

IOResolver::IOResolver() {}

IOResolver::~IOResolver() {}

/*
 * Resolve the var in any nodes before `node`
 * Will search for `decl_stmt` or `params`
 * Return type name if found.
 */
Variable
IOResolver::ResolveLocalVar(
  const std::string& name,
  pugi::xml_node node
) {
  while (node || node.previous_sibling()) {
    node = node.previous_sibling();
    if (node.type() == pugi::node_element) {
      // won't search beyond function. That will resolved as global variable
      if (strcmp(node.name(), "function") == 0) {
        return Variable(false);
      } else if (strcmp(node.name(), "decl_stmt") == 0) {
        Variable v(node);
        if (v.GetName() == name) {
          return v;
        }
      } else if (strcmp(node.name(), "parameter_list") == 0) {
        std::vector<Variable> vv;
        Variable::FromParamList(node, vv);
        for (auto it=vv.begin();it!=vv.end();it++) {
          if (it->GetName() == name) {
            return *it;
          }
        }
      }
    }
  }
  if (node && node.parent()) {
    return ResolveLocalVar(name, node.parent());
  } else {
    return Variable(false);
  }
}
/*
 * Resolve all alive vars at this node.
 */
void IOResolver::ResolveAliveVars(pugi::xml_node node, std::set<Variable>& resolved) {
  if (node && node.previous_sibling()) {
    // go to previous sibling if any
    node = node.previous_sibling();
    if (node.type() == pugi::node_element) {
      if (strcmp(node.name(), "decl_stmt") == 0) {
        Variable v(node);
        resolved.insert(node);
      } else if (strcmp(node.name(), "parameter_list") == 0) {
        std::vector<Variable> vv;
        Variable::FromParamList(node, vv);
        for (auto it=vv.begin();it!=vv.end();it++) {
          resolved.insert(node);
        }
      }
    }
  } else if (node.parent()) {
    // go to parent if no previous sibling
    node = node.parent();
  } else {
    // return if no even parent
    return;
  }
  ResolveAliveVars(node, resolved);
}


// return name used in <expr>
std::vector<std::string>
parseExpr(pugi::xml_node node) {
  if (node && node.type() == pugi::node_element && strcmp(node.name(), "expr") == 0) {
    // TODO
  }
  return std::vector<std::string>();
}

void
IOResolver::visit(
  pugi::xml_node node,
  std::set<Variable>& resolved,
  std::set<Variable>& defined
) {
  if (node.type() == pugi::node_element) {
    if (strcmp(node.name(), "decl_stmt") == 0) {
      Variable v(node);
      defined.insert(v);
    } else if (strcmp(node.name(), "expr") == 0) {
      // in #ifdef, there may be `#elif defined(__sun)`
      // TODO even if __sun is identified as undefined,
      // we can't resolve its type, then we will not report it...
      if (DomUtil::InNode(node, "cpp:ifdef", 2)
      || DomUtil::InNode(node, "cpp:elif", 2)
      || DomUtil::InNode(node, "cpp:ifndef", 2)) {
        return;
      }
      std::vector<std::string> vs = parseExpr(node); // the name used in <expr>
      for (auto it=vs.begin();it!=vs.end();it++) {
        Variable v = ResolveLocalVar(*it, node);
        resolved.insert(v);
      }
    } else if (strcmp(node.name(), "for") == 0) {
      pugi::xml_node decl_node = node.child("init").child("decl");
      // std::vector<Variable> vv;
      // Variable::FromForInit(init_node, vv);
      // for (auto it=vv.begin();it!=vv.end();it++) {
      //   defined.insert(*it);
      // }
      Variable v(decl_node);
      defined.insert(v);
    }
  }
}
void
IOResolver::resolveUndefinedVars(
  std::vector<pugi::xml_node> vn,
  std::set<Variable>& resolved,
  std::set<Variable>& defined
) {
  for (auto it=vn.begin();it!=vn.end();it++) {
    visit(*it, resolved, defined);
    resolveUndefinedVars(*it, resolved, defined);
  }
}
void
IOResolver::resolveUndefinedVars(
  pugi::xml_node node,
  std::set<Variable>& resolved,
  std::set<Variable>& defined
) {
  std::set<Variable> defined_back = defined;
  for (pugi::xml_node n : node.children()) {
    visit(n, resolved, defined);
    resolveUndefinedVars(n, resolved, defined);
  }
  defined = defined_back;
}

void
IOResolver::ResolveUndefinedVars(
  const Segment& segment,
  std::set<Variable>& resolved
) {
  std::set<Variable> defined;
  std::vector<pugi::xml_node> vn = segment.GetNodes();
  for (auto it=vn.begin();it!=vn.end();it++) {
    resolveUndefinedVars(vn, resolved, defined);
  }
}

void
IOResolver::ResolveUndefinedVars(
  pugi::xml_node node,
  std::set<Variable>& resolved
) {
  std::set<Variable> defined;
  resolveUndefinedVars(node, resolved, defined);
}
// get the variable name only
void
IOResolver::GetUndefinedVars(
  const Segment& segment,
  std::vector<std::string>& resolved
) {}
void
IOResolver::GetUndefiendVars(
  pugi::xml_node node,
  std::vector<std::string>& resolved
) {}
