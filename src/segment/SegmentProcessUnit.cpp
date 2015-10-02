#include "segment/SegmentProcessUnit.hpp"
#include "util/DomUtil.hpp"
#include "resolver/IOResolver.hpp"
#include "resolver/Resolver.hpp"
#include "snippet/SnippetRegistry.hpp"
#include "resolver/Ctags.hpp"
#include "resolver/HeaderSorter.hpp"

SegmentProcessUnit::SegmentProcessUnit(const std::string& filename)
: m_filename(filename),
m_segment(std::make_shared<Segment>()),
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

bool
SegmentProcessUnit::IsValid() {
  // check if valid
  // check if the segment is in a funciton prototype that contains enum parameter or return type
  // if the parameter of function is used, I cannot resolve its type.
  // The segment must be inside a function.
  // FIXME maybe the segment itself is not in a enum function,
  // but when doing context search, it may go into a interprocedure that is in enum function
  // So check m_context every time doing context search should be complete solution.
  pugi::xml_node node = m_segment->GetFirstNode();
  if (node && DomUtil::InNode(node, "function", 100)) {
    return true;
  } else {
    // TODO the segment not in function(the function prototype has enum)
    // may also be able to analyze if it doesn't use the function parameters.
    // But, sorry I don't care about you.
    std::cout << "[SegmentProcessUnit::Valid]"
    << "\033[33m"
    << "segment not valid because it is not in a function"
    << "\033[0m"
    << std::endl;
    return false;
  }
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
      // TODO function should be in generate.c
      // TODO segment should be recognized(kept)
      std::cout << "\033[32m" << "interprocedure context search" << "\033[0m" << std::endl;
      m_functions.push_back(tmp);
      tmp = DomUtil::GetFunctionCall(tmp);
      if (!tmp) return false;
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
  std::cout<<"[SegmentProcessUnit::resolveInput]"<<std::endl;
  // TODO input variable type may be structure. These structure should appear in support.h
  // std::set<Variable> vv;
  m_inv.clear();
  m_inv = IOResolver::ResolveUndefinedVars(*m_segment);
}
void SegmentProcessUnit::resolveOutput() {
  // std::set<Variable> vv;
  // IOResolver::ResolveAliveVars(*m_context, vv);
  // TODO point of interest
}
void SegmentProcessUnit::resolveSnippets() {
  std::cout<<"[SegmentProcessUnit][resolveSnippets]"<<std::endl;
  m_snippets.clear();
  // the initial code to resolve is: context + input variable(input code)
  std::string code = m_context->GetText();
  code += getInputCode();
  std::set<std::string> ss = Resolver::ExtractToResolve(code);
  // std::cout << "size of to resolve: " << ss.size() << std::endl;
  for (auto it=ss.begin();it!=ss.end();it++) {
    std::set<Snippet*> snippets = Ctags::Instance()->Resolve(*it);
    m_snippets.insert(snippets.begin(), snippets.end());
  }
  std::cout<<"[SegmentProcessUnit][resolveSnippets] size: " << m_snippets.size() <<std::endl;
}

// true if s1 is direct dep of s2
bool
isDirectDep(Snippet* s1, Snippet* s2) {
  std::set<Snippet*> dep = SnippetRegistry::Instance()->GetDependence(s2);
  if (dep.find(s1) == dep.end()) return true;
  else return false;
}

// return true if have something changed
bool
sortOneRound(std::vector<Snippet*> &sorted) {
  std::cout << "sortOneRound" << std::endl;
  for (auto it=sorted.begin();it!=sorted.end();it++) {
    std::cout << "\t" << (*it)->GetName() << std::endl;
  }
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

bool
compare(const Snippet* s1, const Snippet* s2) {
  return (s1->GetLineNumber() < s2->GetLineNumber());
}

// use HeaderSorter
std::vector<Snippet*>
sortSnippets2(std::set<Snippet*> all) {
  // std::cout << "sortSnippets2" << std::endl;
  std::vector<Snippet*> sorted;
  // prepare containers
  // std::cout << "prepare containers" << std::endl;
  std::set<std::string> all_files;
  std::map<std::string, std::vector<Snippet*> > file_to_snippet_map;
  for (auto it=all.begin();it!=all.end();it++) {
    std::string filename = (*it)->GetFilename();
    all_files.insert(filename);
    if (file_to_snippet_map.find(filename) == file_to_snippet_map.end()) {
      file_to_snippet_map[filename] = std::vector<Snippet*>();
    }
    file_to_snippet_map[filename].push_back(*it);
  }
  // sort the file_to_snippet_map
  for (auto it=file_to_snippet_map.begin();it!=file_to_snippet_map.end();it++) {
    std::sort(it->second.begin(), it->second.end(), compare);
  }

  // to make the c files always at the bottom
  std::set<std::string> all_h_files;
  std::set<std::string> all_c_files;
  for (auto it=all_files.begin();it!=all_files.end();it++) {
    if ((*it).back() == 'c') {all_c_files.insert(*it);}
    else if ((*it).back() == 'h') {all_h_files.insert(*it);}
    else {std::cout << "[sortSnippets2] Warning: not c or h file" << std::endl;}
  }

  // sort file names
  std::vector<std::string> sorted_files = HeaderSorter::Instance()->Sort(all_h_files);
  std::copy(all_c_files.begin(), all_c_files.end(), std::back_inserter(sorted_files));
  for (auto it=sorted_files.begin();it!=sorted_files.end();it++) {
    for (auto it2=file_to_snippet_map[*it].begin();it2!=file_to_snippet_map[*it].end();it2++) {
      sorted.push_back(*it2);
    }
  }
  return sorted;
}

std::string
SegmentProcessUnit::getContext() {
  std::string context = m_context->GetText();
  std::regex return_regex("\\breturn\\b[^;]*;");
  return std::regex_replace(context, return_regex, "//replaced return\n");
}

std::string
SegmentProcessUnit::getInputCode() {
  std::string s;
  for (auto it=m_inv.begin();it!=m_inv.end();it++) {
    s += (*it)->GetInputCode();
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
  s += "// Input\n";
  s += getInputCode();
  s += "// Context\n";
  s += "// " + m_filename + "\n";
  // s += m_context->GetText();
  s += getContext();
  s += "\n}";
  return s;
}

std::string
get_headers() {
  std::vector<std::string> headers{
    "stdio.h",
    "stdlib.h",
    "stdint.h",
    "sys/types.h",
    "assert.h",
    "pthread.h",
    "string.h",
    "signal.h",
    "errno.h",
    "unistd.h",
    "fcntl.h",
    "ctype.h",
    "sys/wait.h",
    "time.h",
    // linux
    "errno.h",
    // network
    "sys/socket.h",
    "sys/un.h",
    "netinet/in.h",
    "netinet/tcp.h",
    "arpa/inet.h",
    // unfamiliar
    "sys/stat.h",
    "sys/uio.h",
    "sys/param.h",
    "sys/mman.h",
    "netdb.h",
    "sys/stat.h",
    "sys/param.h",
    "sys/resource.h",
    "stddef.h",
    // FIXME 3rd party. Need to install first!!!
    "event.h"
  };
  std::string code;
  for (int i=0;i<headers.size();i++) {
    code += "#include \"" + headers[i] + "\"\n";
  }
  return code;
}

std::string
get_head() {
  return
  "#ifndef __SUPPORT_H__\n"
  "#define __SUPPORT_H__\n"
  "typedef int bool;\n"
  "#define true 1\n"
  "#define false 0\n"
  // some code will config to have this variable.
  // Should not just define it randomly, but this is the best I can do to make it compile ...
  "#define VERSION \"1\"\n"
  "#define PACKAGE \"helium\"\n"
  ;
}

std::string
get_foot() {
  return std::string()
  + "\n#endif\n";
}

std::string
SegmentProcessUnit::GetSupport() {
  std::cout << "[SegmentProcessUnit::GetSupport]" << std::endl;
  // prepare the containers
  std::set<Snippet*> all_snippets;
  all_snippets = SnippetRegistry::Instance()->GetAllDependence(m_snippets);
  std::cout << "[SegmentProcessUnit::GetSupport] all snippets: " << all_snippets.size() << std::endl;
  // sort the snippets
  std::vector<Snippet*> sorted_all_snippets = sortSnippets2(all_snippets);
  std::cout << "after sort snippet: " << sorted_all_snippets.size() << std::endl;
  // return the snippet code
  std::string code = "";
  // head
  code += get_head();
  code += get_headers();
  // snippets
  std::string code_func_decl;
  std::string code_func;
  for (auto it=sorted_all_snippets.begin();it!=sorted_all_snippets.end();it++) {
    if ((*it)->GetType() == 'f') {
      // functions
      code_func_decl += (*it)->GetDecl()+"\n";
      code_func += (*it)->GetCode()+"\n";
    } else {
      // all other codes
      code +=
      "// " + (*it)->GetFilename() + ":" + std::to_string((*it)->GetLineNumber())
      + "\n" + (*it)->GetCode() + '\n';
    }
  }
  code += "\n// function declarations\n";
  code += code_func_decl;
  code += "\n// functions\n";
  code += code_func;
  // foot
  code += get_foot();
  return code;
}

std::string
SegmentProcessUnit::GetMakefile() {
  std::string makefile;
  makefile = makefile + "a.out: generate.c\n"
  // makefile += "\tcc -std=c99 generate.c " + compile_option +"\n"
  // FIXME The -levent is 3rd party! Need to install first!
  + "\tcc -std=c99 generate.c " + "-levent" + "\n"
  + "clean:\n"
  + "\trm -rf *.out";
  return makefile;
}
