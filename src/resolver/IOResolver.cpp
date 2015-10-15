#include "resolver/IOResolver.hpp"
#include "util/DomUtil.hpp"
#include "variable/VariableFactory.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>
#include "Logger.hpp"

IOResolver::IOResolver() {}

IOResolver::~IOResolver() {}

/*
 * Resolve the var in any nodes before `node`
 * Will search for `decl_stmt` or `params`
 * Return type name if found.
 */
std::shared_ptr<Variable>
IOResolver::ResolveLocalVar(
  const std::string& name,
  pugi::xml_node node
) {
  // std::cout<<"[IOResolver::ResolveLocalVar] "<<name<<std::endl;
  while (node && node.previous_sibling()) {
    node = node.previous_sibling();
    if (node.type() == pugi::node_element) {
      // won't search beyond function. That will resolved as global variable
      // FIXME: slabs_clsid in slabs.c in memcached will skip the <function> ...
      // if (name == "slabclass") {
      //   std::cout << "\033[32m" << "slab class" << "\033[0m" << std::endl;
      //   std::cout << node.name() << std::endl;
      //   std::cout << DomUtil::GetTextContent(node) << std::endl;
      // }
      if (strcmp(node.name(), "function") == 0) {
        return NULL;
      } else if (strcmp(node.name(), "decl_stmt") == 0) {
        std::shared_ptr<Variable> v = VariableFactory::FromDeclStmt(node);
        if (v && v->GetName() == name) {
          return v;
        }
      } else if (strcmp(node.name(), "parameter_list") == 0) {
        std::vector<std::shared_ptr<Variable> > vv = VariableFactory::FromParamList(node);
        for (auto it=vv.begin();it!=vv.end();it++) {
          if ((*it)->GetName() == name) {
            return *it;
          }
        }
      }
    }
  }
  if (node && node.parent()) {
    return ResolveLocalVar(name, node.parent());
  } else {
    return NULL;
  }
}
/*
 * Resolve all alive vars at this node.
 */
void IOResolver::ResolveAliveVars(
  pugi::xml_node node,
  std::set<std::shared_ptr<Variable> >& resolved
) {
  if (node && node.previous_sibling()) {
    // go to previous sibling if any
    node = node.previous_sibling();
    if (node.type() == pugi::node_element) {
      if (strcmp(node.name(), "decl_stmt") == 0) {
        std::shared_ptr<Variable> vp = VariableFactory::FromDeclStmt(node);
        resolved.insert(vp);
      } else if (strcmp(node.name(), "parameter_list") == 0) {
        std::vector<std::shared_ptr<Variable> > vv = VariableFactory::FromParamList(node);
        for (auto it=vv.begin();it!=vv.end();it++) {
          resolved.insert(*it);
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

void
simplify_variable_name(std::string& s) {
  s = s.substr(0, s.find('['));
  s = s.substr(0, s.find("->"));
  s = s.substr(0, s.find('.'));
  // TODO wiki the erase-remove-idiom
  s.erase(std::remove(s.begin(), s.end(), '('), s.end());
  s.erase(std::remove(s.begin(), s.end(), ')'), s.end());
  s.erase(std::remove(s.begin(), s.end(), '*'), s.end());
  s.erase(std::remove(s.begin(), s.end(), '&'), s.end());
}
// return name used in <expr>
std::vector<std::string>
parseExpr(pugi::xml_node node) {
  std::vector<std::string> vs;
  if (node && node.type() == pugi::node_element && strcmp(node.name(), "expr") == 0) {
    for (pugi::xml_node n : node.children("name")) {
      std::string s = DomUtil::GetTextContent(n);
      simplify_variable_name(s);
      vs.push_back(s);
    }
    return vs;
  }
  return std::vector<std::string>();
}

bool
var_in_set(const std::string& name, std::set<std::shared_ptr<Variable> >& s) {
  for (auto it=s.begin();it!=s.end();it++) {
    if ((*it)->GetName() == name) return true;
  }
  return false;
}
void
IOResolver::visit(
  pugi::xml_node node,
  std::set<std::shared_ptr<Variable> >& resolved,
  std::set<std::shared_ptr<Variable> >& defined
) {
  if (node.type() == pugi::node_element) {
    if (strcmp(node.name(), "decl_stmt") == 0) {
      std::shared_ptr<Variable> vp = VariableFactory::FromDeclStmt(node);
      if (vp) defined.insert(vp);
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
        // check if in resolved or defined
        if (var_in_set(*it, resolved) || var_in_set(*it, defined)) {
          continue;
        }
        std::shared_ptr<Variable> vp = ResolveLocalVar(*it, node);
        if (vp) {
          resolved.insert(vp);
        }
      }
    } else if (strcmp(node.name(), "for") == 0) {
      pugi::xml_node decl_node = node.child("init").child("decl");
      std::shared_ptr<Variable> vp = VariableFactory::FromDecl(decl_node);
      if (vp) defined.insert(vp);
    }
  }
}

void
IOResolver::resolveUndefinedVars(
  std::vector<pugi::xml_node> nodes,
  std::set<std::shared_ptr<Variable> >& resolved,
  std::set<std::shared_ptr<Variable> > defined // copy!
) {
  // caution: this is a performance bottle neck.
  // to limit the segment size may help
  // static int count = 0;
  // count++;
  // std::cout << "[IOResolver::resolveUndefinedVars] " << count << std::endl;
  for (auto it=nodes.begin();it!=nodes.end();it++) {
    visit(*it, resolved, defined);
    std::vector<pugi::xml_node> vn;
    for (pugi::xml_node n : it->children()) {
      vn.push_back(n);
    }
    resolveUndefinedVars(vn, resolved, defined);
  }
}

std::set<std::shared_ptr<Variable> >
IOResolver::ResolveUndefinedVars(
  const Segment& segment
) {
  Logger::Instance()->LogTrace("[IOResolver::ResolveUndefinedVars]\n");
  std::set<std::shared_ptr<Variable> > resolved;
  std::set<std::shared_ptr<Variable> > defined;
  std::vector<pugi::xml_node> vn = segment.GetNodes();
  resolveUndefinedVars(vn, resolved, defined);
  return resolved;
}

std::set<std::shared_ptr<Variable> >
IOResolver::ResolveUndefinedVars(
  pugi::xml_node node
) {
  Logger::Instance()->LogTrace("[IOResolver::ResolveUndefinedVars]\n");
  std::set<std::shared_ptr<Variable> > resolved;
  std::set<std::shared_ptr<Variable> > defined;
  std::vector<pugi::xml_node> vn;
  vn.push_back(node);
  resolveUndefinedVars(vn, resolved, defined);
  return resolved;
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
