#include "segment/SegmentProcessUnit.hpp"
#include "util/DomUtil.hpp"
#include "resolver/IOResolver.hpp"
#include "resolver/Resolver.hpp"
#include "snippet/SnippetRegistry.hpp"
#include "resolver/Ctags.hpp"
#include "resolver/HeaderSorter.hpp"
#include "resolver/SystemResolver.hpp"
#include <cstring>
#include <algorithm>
#include <boost/regex.hpp>
#include "Logger.hpp"

SegmentProcessUnit::SegmentProcessUnit(const std::string& filename)
: m_filename(filename),
m_segment(std::make_shared<Segment>()),
m_context(std::make_shared<Segment>()),
m_linear_search_value(0) {
  Logger::Instance()->LogTrace("[SegmentProcessUnit][Constructor]\n");
}
SegmentProcessUnit::~SegmentProcessUnit() {}

void SegmentProcessUnit::SetSegment(const Segment &s) {
  m_segment = std::make_shared<Segment>(s);
}
void SegmentProcessUnit::AddNode(pugi::xml_node node) {
  m_segment->PushBack(node);
}
void SegmentProcessUnit::Process() {
  Logger::Instance()->LogTrace("[SegmentProcessUnit] Processing Segment Unit\n");
  m_context = m_segment;
  resolveInput();
  resolveOutput();
  resolveSnippets();
}

bool
SegmentProcessUnit::IsValid() {
  // check if valid
  // check segment size
  std::string content = m_segment->GetText();
  int size = std::count(content.begin(), content.end(), '\n');
  if (size > Config::Instance()->GetMaxSegmentSize()) {
    Logger::Instance()->LogTrace("[SegmentProcessUnit::IsValid]"
    "[WARNING]"
    "segment not valid because its size: "
    + std::to_string(size)
    + " is larger than max: "
    + std::to_string(Config::Instance()->GetMaxSegmentSize()) +"\n");
    return false;
  }
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
    Logger::Instance()->LogTrace("[SegmentProcessUnit::Valid]"
    "[WARNING]"
    "segment not valid because it is not in a function\n");
    return false;
  }
}

bool SegmentProcessUnit::IncreaseContext() {
  Logger::Instance()->LogTrace("[SegmentProcessUnit][IncreaseContext]");
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
      Logger::Instance()->LogTrace("[SegmentProcessUnit::IncreaseContext]"
      "interprocedure context search\n");
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
  Logger::Instance()->LogTrace("[SegmentProcessUnit::resolveInput]\n");
  // TODO input variable type may be structure. These structure should appear in support.h
  // std::set<Variable> vv;
  m_inv.clear();
  m_inv = IOResolver::ResolveUndefinedVars(*m_segment);
}

pugi::xml_node
get_outmost_loop_node(std::shared_ptr<Segment> segment) {
  std::vector<pugi::xml_node> nodes = segment->GetNodes();
  for (auto it=nodes.begin();it!=nodes.end();it++) {
    if (strcmp(it->name(), "for") == 0 || strcmp(it->name(), "while") == 0) {
      return *it;
    }
  }
  return pugi::xml_node();
}

void
SegmentProcessUnit::instrument() {
  std::string instrument_position = Config::Instance()->GetInstrumentPosition();
  std::string instrument_type = Config::Instance()->GetInstrumentType();
  if (instrument_position.empty() || instrument_type.empty()) return;
  if (instrument_position == "loop") {
    pugi::xml_node loop_node = get_outmost_loop_node(m_segment);
    pugi::xml_node block_node = loop_node.child("block");
    if (block_node) {
      // create only <helium_instrument> node, for latter remove
      pugi::xml_node new_node = block_node.insert_child_before(
        "helium_instrument", block_node.last_child()
      );
      new_node.append_child(pugi::node_pcdata).set_value("\n// @Output\n");
      m_output_node = new_node;
    }
  }
}

void
SegmentProcessUnit::uninstrument() {
  // pugi::xml_node seg_parent_node = m_segment->GetFirstNode().parent();
  // pugi::xpath_node_set helium_instruments = seg_parent_node.select_nodes("//helium_instrument");
  // for (auto it=helium_instruments.begin();it!=helium_instruments.end();it++) {
  //   pugi::xml_node tmp = it->parent();
  //   tmp.remove_child(it->node());
  // }
  if (m_output_node) {
    pugi::xml_node tmp = m_output_node.parent();
    tmp.remove_child(m_output_node);
    m_output_node = pugi::xml_node();
  }
}

void SegmentProcessUnit::resolveOutput() {
  Logger::Instance()->LogTrace("[SegmentProcessUnit::resolveOutput]\n");
  // std::set<Variable> vv;
  // TODO point of interest
  uninstrument();
  instrument();
  m_outv.clear();
  if (m_output_node) {
    // IOResolver::ResolveAliveVars(m_output_node, m_outv);
    // TODO resolveAliveVars
    // TODO until a node(the segment start node)
    m_outv = m_inv;
    for (auto it=m_outv.begin();it!=m_outv.end();it++) {
      pugi::xml_node node = m_output_node.append_child("outv");
      node.append_child(pugi::node_pcdata).set_value((*it)->GetOutputCode().c_str());
    }
  }
}

void SegmentProcessUnit::resolveSnippets() {
  Logger::Instance()->LogTrace("[SegmentProcessUnit][resolveSnippets]\n");
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
  Logger::Instance()->LogTrace("[SegmentProcessUnit][resolveSnippets] size: "
  + std::to_string(m_snippets.size()) + "\n");
}

bool
compare(const Snippet* s1, const Snippet* s2) {
  return (s1->GetLineNumber() < s2->GetLineNumber());
}

// use HeaderSorter
std::vector<Snippet*>
sortSnippets(std::set<Snippet*> all) {
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
  boost::regex return_regex("\\breturn\\b[^;]*;");
  context = boost::regex_replace<boost::regex_traits<char>, char>(context, return_regex, ";//replaced return\n");
  // FIXME when doing context search, break may appear, while we don't have the outside loop
  return context;
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
  // GNU Standard
  "#define PACKAGE_NAME \"helium\"\n"
  "#define PACKAGE_BUGREPORT \"xxx@xxx.com\"\n"
  "#define PACKAGE_STRING \"helium 0.1\"\n"
  "#define PACKAGE_TARNAME \"helium\"\n"
  "#define PACKAGE_URL \"\"\n"
  "#define PACKAGE_VERSION \"0.1\"\n"
  ;
}

std::string
get_foot() {
  return std::string()
  + "\n#endif\n";
}

std::string
SegmentProcessUnit::GetSupport() {
  Logger::Instance()->LogTrace("[SegmentProcessUnit::GetSupport]\n");
  // prepare the containers
  std::set<Snippet*> all_snippets;
  all_snippets = SnippetRegistry::Instance()->GetAllDependence(m_snippets);
  Logger::Instance()->LogTrace("[SegmentProcessUnit::GetSupport] all snippets: "
  + std::to_string(all_snippets.size()) + "\n");
  if (all_snippets.size() > Config::Instance()->GetMaxSnippetNumber()) {
    Logger::Instance()->LogWarning(
      "[SegmentProcessUnit::GetSupport] snippet number larger than config: "
      + std::to_string(all_snippets.size()) +">"
      + std::to_string(Config::Instance()->GetMaxSnippetNumber())
    );
    m_can_continue = false;
  }
  // sort the snippets
  std::vector<Snippet*> sorted_all_snippets = sortSnippets(all_snippets);
  Logger::Instance()->LogTrace("after sort snippet: "
  + std::to_string(sorted_all_snippets.size()) + "\n");
  // return the snippet code
  std::string code = "";
  // head
  code += get_head();
  code += SystemResolver::Instance()->GetHeaders();
  // snippets
  std::string code_func_decl;
  std::string code_func;
  for (auto it=sorted_all_snippets.begin();it!=sorted_all_snippets.end();it++) {
    if ((*it)->GetType() == 'f') {
      // functions
      code_func_decl += (*it)->GetDecl()+"\n";
      code_func +=
      "// " + (*it)->GetFilename() + ":" + std::to_string((*it)->GetLineNumber())
      + "\n" + (*it)->GetCode() + '\n';
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
  // FIXME library should be changed according to CondComp
  // TODO configurable include paths
  + "\tcc -std=c99 generate.c " + SystemResolver::Instance()->GetLibs() + "\n"
  + "clean:\n"
  + "\trm -rf *.out";
  return makefile;
}
