#include "segment/SegmentProcessUnit.hpp"
#include "util/DomUtil.hpp"
#include "resolver/IOResolver.hpp"
#include "resolver/Resolver.hpp"
#include "snippet/SnippetRegistry.hpp"

SegmentProcessUnit::SegmentProcessUnit()
: m_segment(std::make_shared<Segment>()),
m_context(std::make_shared<Segment>()),
m_linear_search_value(0) {
  std::cout<<"[SegmentProcessUnit][Constructor]"<<std::endl;
}
SegmentProcessUnit::~SegmentProcessUnit() {}

void SegmentProcessUnit::SetSegment(const Segment &s) {
  m_segment = std::make_shared<Segment>(s);
}
void SegmentProcessUnit::AddNode(pugi::xml_node node) {
  m_segment->PushBack(node);
}
void SegmentProcessUnit::Process() {
  std::cout<<"[SegmentProcessUnit] Processing Segment Unit"<<std::endl;
  m_context = m_segment;
  resolveInput();
  resolveOutput();
  resolveSnippets();
}

bool SegmentProcessUnit::IncreaseContext() {
  std::cout<<"[SegmentProcessUnit][IncreaseContext]"<<std::endl;
  m_linear_search_value++;
  if (m_linear_search_value > Config::Instance()->GetMaxLinearSearchValue()) {
    return false;
  }
  // increase context
  pugi::xml_node first_node = m_context->GetNodes()[0];
  pugi::xml_node tmp = DomUtil::GetPreviousASTElement(first_node);
  if (tmp) {
    // sibling nodes
    m_context->PushFront(tmp);
  } else {
    tmp = DomUtil::GetParentASTElement(first_node);
    if (!tmp) return false;
    if (strcmp(tmp.name(), "function") == 0) {
      // interprocedure
      m_functions.push_back(tmp);
      tmp = DomUtil::GetFunctionCall(tmp);
      m_context->Clear();
      m_context->PushBack(tmp);
    } else {
      // parent node
      m_context->Clear();
      m_context->PushBack(tmp);
    }
  }
  resolveInput();
  resolveOutput();
  resolveSnippets();
  return true;
}

void SegmentProcessUnit::resolveInput() {
  // std::set<Variable> vv;
  m_inv.clear();
  IOResolver::ResolveUndefinedVars(*m_segment, m_inv);
}
void SegmentProcessUnit::resolveOutput() {
  // std::set<Variable> vv;
  // IOResolver::ResolveAliveVars(*m_context, vv);
  // TODO point of interest
}
void SegmentProcessUnit::resolveSnippets() {
  std::cout<<"[SegmentProcessUnit][resolveSnippets]"<<std::endl;
  m_snippets.clear();
  std::string code = m_context->GetText();
  Resolver resolver = Resolver(code);
  resolver.Resolve();
  m_snippets = resolver.GetSnippets();
  // TODO
}

// true if s1 is direct dep of s2
bool
isDirectDep(Snippet* s1, Snippet* s2) {
  std::set<Snippet*> dep = SnippetRegistry::Instance()->GetDependence(s2);
  if (dep.find(s1) == dep.end()) return false;
  else return true;
}

// return true if have something changed
bool
sortOneRound(std::vector<Snippet*> &sorted) {
  bool changed = false;
  for (int i=0;i<sorted.size();i++) {
    for (int j=i+1;j<sorted.size();j++) {
      if (isDirectDep(sorted[i], sorted[j])) {
        Snippet *tmp = sorted[i];
        sorted[i] = sorted[j];
        sorted[j] = tmp;
        changed = true;
      }
    }
  }
  return changed;
}

std::vector<Snippet*>
sortSnippets(std::set<Snippet*> ss) {
  std::vector<Snippet*> sorted;
  for (auto it=ss.begin();it!=ss.end();it++) {
    sorted.push_back(*it);
  }
  while (sortOneRound(sorted)) ;
  return sorted;
}


std::string
SegmentProcessUnit::InstrumentIO() {
  return "";
}
std::string
SegmentProcessUnit::GetContext() {
  return "";
}

std::string
SegmentProcessUnit::getInputCode() {
  std::string s;
  for (auto it=m_inv.begin();it!=m_inv.end();it++) {
    s += it->GetInputCode();
  }
  return s;
}
std::string
getHeader() {
  std::string s;
  std::vector<std::string> system_headers;
  std::vector<std::string> local_headers;
  system_headers.push_back("stdio.h");
  local_headers.push_back("support.h");
  for (auto it=system_headers.begin();it!=system_headers.end();it++) {
    s += "#include <" + *it + ">\n";
  }
  for (auto it=local_headers.begin();it!=local_headers.end();it++) {
    s += "#include \"" + *it + "\"\n";
  }
  return s;
}
std::string
SegmentProcessUnit::GetMain() {
  std::string s;
  s += getHeader();
  s += "int main() {\n";
  s += getInputCode();
  s += m_context->GetText();
  s += "\n}";
  return s;
}

std::string
SegmentProcessUnit::GetSupport() {
  return "";
  // prepare the containers
  std::set<Snippet*> all_snippets;
  std::set<Snippet*> to_resolve;
  for (auto it=m_snippets.begin();it!=m_snippets.end();it++) {
     all_snippets.insert(*it);
     to_resolve.insert(*it);
  }
  // get all snippets
  while (!to_resolve.empty()) {
    Snippet *tmp = *(to_resolve.begin());
    to_resolve.erase(tmp);
    all_snippets.insert(tmp);
    std::set<Snippet*> dep = SnippetRegistry::Instance()->GetDependence(tmp);
    for (auto it=dep.begin();it!=dep.end();it++) {
      if (all_snippets.find(*it) == all_snippets.end()) {
        // not found, new snippet
        to_resolve.insert(*it);
      }
    }
  }
  // sort the snippets
  std::vector<Snippet*> sorted_all_snippets = sortSnippets(all_snippets);
  // return the snippet code
  std::string code = "";
  for (auto it=sorted_all_snippets.begin();it!=sorted_all_snippets.end();it++) {
    code += (*it)->GetCode();
  }
  return code;
}

std::string
SegmentProcessUnit::GetMakefile() {
  std::string makefile;
  makefile = makefile + "a.out: generate.c\n"
  // makefile += "\tcc -std=c99 generate.c " + compile_option +"\n"
  + "\tcc -std=c99 generate.c " + "\n"
  + "clean:\n"
  + "\trm -rf *.out";
  return makefile;
}
