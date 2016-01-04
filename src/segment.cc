#include "segment.h"
#include "utils.h"
#include <iostream>
#include "resolver.h"

#include "config.h"

using namespace ast;

Segment::Segment () {}
Segment::~Segment () {}

void Segment::PushBack(Node node) {
  m_nodes.PushBack(node);
  updateMeta();
}
void Segment::PushBack(NodeList nodes) {
  m_nodes.PushBack(nodes);
  updateMeta();
}
void Segment::PushFront(Node node) {
  m_nodes.PushFront(node);
  updateMeta();
}

void Segment::PushFront(NodeList nodes) {
  m_nodes.PushFront(nodes);
  updateMeta();
}

void Segment::updateMeta() {
  std::string text = GetText();
  m_loc = std::count(text.begin(), text.end(), '\n');
}
void Segment::Clear() {
  m_nodes.Clear();
  m_loc = 0;
}

int Segment::GetLineNumber() const {
  for (Node n : m_nodes.Nodes()) {
    // FIXME 0 is a magic number! The acutal value inside Node is -1 ..
    if (n.GetFirstLineNumber() > 0) return n.GetFirstLineNumber();
  }
  return 0;
}

NodeList Segment::GetNodes() const {
  return m_nodes;
}

Node
Segment::GetFirstNode() const {
  if (m_nodes.Empty()) {
    // this should be a node_null
    return Node();
  } else {
    return m_nodes.Get(0);
  }
}

std::string
Segment::GetText() {
  std::string s;
  for (Node node : m_nodes.Nodes()) {
    s += node.Text();
    // s += '\n'; // FIXME need this?
  }
  return s;
}

std::string
Segment::GetTextExceptComment() {
  std::string s;
  for (Node node : m_nodes.Nodes()) {
    s += node.TextExceptComment();
  }
  return s;
}

bool
Segment::HasNode(Node node) const {
  return m_nodes.Contains(node);
}

void Segment::Grow() {
  Node first_node = m_nodes.Get(0);
  Node n = first_node.PreviousSibling();
  // has previous sibling
  if (n) {
    this->PushFront(n);
    m_valid = true;
    goto after;
  }
  n = first_node.Parent();
  // don't have parent
  if (!n) {
    m_valid = false;
    return;
  }
  // parent is function
  if (n.Type() == NK_Function) {
    m_function_nodes.PushBack(n);
    n = get_function_call(n);
    // can't get function callsite
    if (!n) {
      m_valid = false;
      return;
    }
  }
  this->Clear();
  this->PushFront(n);
 after:
  // TODO check size of segment
  return;
}


/*******************************
 ** SPU
 *******************************/

SPU::SPU(const std::string& filename) : m_filename(filename) {}
SPU::~SPU() {
  // remove instrument when deconstruct
  unsimplifyCode();
  // Techniquely it should uninstrument here, but I'm encountering with a pugixml bug:
  // node.parent().remove_child(node) will get error.
  // uninstrument();
}

void SPU::SetSegment(const Segment &s) {
  m_segment = s;
}
void SPU::AddNode(Node node) {
  m_segment.PushBack(node);
}
void SPU::AddNodes(NodeList nodes) {
  m_segment.PushBack(nodes);
}
void SPU::Process() {
  m_context = m_segment;
  // doing library call experiment
  resolveInput();
  resolveOutput();
  // do not need resolveSnippet at this point
  // resolveSnippets();
}

bool
SPU::IsValid() {
  // check if valid
  // check segment size
  std::string content = m_segment.GetText();
  int size = std::count(content.begin(), content.end(), '\n');
  if (size > Config::Instance()->GetInt("max_segment_size")) {
    return false;
  }
  // check if the segment is in a funciton prototype that contains enum parameter or return type
  // if the parameter of function is used, I cannot resolve its type.
  // The segment must be inside a function.
  // FIXME maybe the segment itself is not in a enum function,
  // but when doing context search, it may go into a interprocedure that is in enum function
  // So check m_context every time doing context search should be complete solution.
  Node node = m_segment.GetFirstNode();
  if (node && in_node(node, NK_Function)) {
    return true;
  } else {
    // TODO the segment not in function(the function prototype has enum)
    // may also be able to analyze if it doesn't use the function parameters.
    // But, sorry I don't care about you.
    return false;
  }
}

bool SPU::IncreaseContext() {
  m_linear_search_value++;
  if (m_linear_search_value > Config::Instance()->GetInt("context_search_value")) {
    return false;
  }
  m_context.Grow();
  resolveInput();
  resolveOutput();
  resolveSnippets();
  return true;
}

void SPU::resolveInput() {
  // TODO input variable type may be structure. These structure should appear in support.h
  // std::set<Variable> vv;
  m_inv.Clear();
  resolver::get_undefined_vars(m_context.GetNodes(), m_inv);
}

/**
 * Recursively determine if the node and its children may influence key
 */
// void
// SPU::doSimplifyCode(Node node, Node key) {
//   if (DomUtil::lub(node, key) == node) {
//     // node is the ancestor of key
//     for (Node n : node.children()) {
//       doSimplifyCode(n, key);
//     }
//   } else if (strcmp(node.name(), "then") == 0 ||
//     strcmp(node.name(), "else") == 0 ||
//     strcmp(node.name(), "elseif") == 0 ||
//     strcmp(node.name(), "case") == 0 ||
//     strcmp(node.name(), "default") == 0
//   ) {
//     // cannot just use AST nodes because <condition> is also a node, but it is not a node in CFG
//     // so we enumerate the branches which are able to eliminate: if <then> <else> <elseif>, switch <case> <default>
//     // node is in different branch
//     Node ancestor = DomUtil::lub(node, key);
//     while (m_context->HasNode(ancestor)) {
//       // here we can only consider the possible loop back edge(without goto)
//       // because we only have AST info, no semantic.
//       // specifically, we have <while> <do> <for>
//       if(strcmp(ancestor.name(), "while") == 0 ||
//         strcmp(ancestor.name(), "do") == 0 ||
//         strcmp(ancestor.name(), "for") == 0
//       ) {
//         return;
//       }
//       ancestor = ancestor.parent();
//     }
//     // no loop ancestor in the context
//     // the mark attribute
//     node.append_attribute("helium-omit");
//     // std::string tmp = get_text_content(node);
//     m_omit_nodes.push_back(node);
//   }
// }

// void
// SPU::simplifyCode() {
//   // remove the AST branches in the context that has no influence on the segment
//   std::vector<Node> nodes = m_context->GetNodes();
//   Node segment_node = m_segment->GetFirstNode();

//   for (auto it=nodes.begin();it!=nodes.end();it++) {
//     doSimplifyCode(*it, segment_node);
//   }
// }

// void
// SPU::unsimplifyCode() {
//   for (auto it=m_omit_nodes.begin();it!=m_omit_nodes.end();it++) {
//     it->remove_attribute(it->attribute("helium-omit"));
//   }
// }

/*******************************
 ** TODO instrument
 *******************************/
// Node
// get_outmost_loop_node(Segment segment) {
//   NodeList nodes = segment.GetNodes();
//   for (auto it=nodes.begin();it!=nodes.end();it++) {
//     if (strcmp(it->name(), "for") == 0 || strcmp(it->name(), "while") == 0) {
//       return *it;
//     }
//   }
//   return Node();
// }

// void
// SPU::instrument() {
//   std::string instrument_position = Config::Instance()->GetInstrumentPosition();
//   std::string instrument_type = Config::Instance()->GetInstrumentType();
//   if (instrument_position.empty() || instrument_type.empty()) return;
//   if (instrument_position == "loop") {
//     Node loop_node = get_outmost_loop_node(m_segment);
//     Node block_node = loop_node.child("block");
//     if (block_node) {
//       // create only <helium_instrument> node, for latter remove
//       Node new_node = block_node.insert_child_before(
//         "helium_instrument", block_node.last_child()
//       );
//       new_node.append_child(pugi::node_pcdata).set_value("\n// @Output\n");
//       m_output_node = new_node;
//     }
//   }
// }

// void
// SPU::uninstrument() {
//   // Node seg_parent_node = m_segment.GetFirstNode().parent();
//   // pugi::xpath_node_set helium_instruments = seg_parent_node.select_nodes("//helium_instrument");
//   // for (auto it=helium_instruments.begin();it!=helium_instruments.end();it++) {
//   //   Node tmp = it->parent();
//   //   tmp.remove_child(it->node());
//   // }
//   if (m_output_node) {
//     Node tmp = m_output_node.parent();
//     // this remove_child will cause error!!! crash!
//     tmp.remove_child(m_output_node);
//     m_output_node = Node();
//   }
// }


void SPU::resolveOutput() {
  // std::set<Variable> vv;
  // TODO point of interest
  // uninstrument();
  // instrument();
  m_outv.Clear();
  if (m_output_node) {
    resolver::get_alive_vars(m_output_node, m_context.GetNodes(), m_outv);
    // if (Config::Instance()->WillSimplifyOutputVar()) {
    //   // remove already used vars in the segment/context
    //   // m_context->GetNodes();
    //   std::string context = getContext();
    //   remove_used_vars(context, m_outv);
    // }
    /* TODO output instrument */
    // for (auto it=m_outv.begin();it!=m_outv.end();it++) {
    //   Node node = m_output_node.append_child("outv");
    //   node.append_child(pugi::node_pcdata).set_value((*it)->GetOutputCode().c_str());
    // }
  }
}

void SPU::resolveSnippets() {
  m_snippets.clear();
  // the initial code to resolve is: context + input variable(input code)
  // std::string code = m_context->GetText();
  // Now we assume the comments exist in the code (no longer do the remove comment preprocessing)
  // We should not resolve the words in comments as identifiers
  std::string code = m_context.GetTextExceptComment();
  code += getInputCode();
  // TODO use semantic when resolving
  std::set<std::string> ids = extract_id_to_resolve(code);
  for (auto it=ids.begin();it!=ids.end();++it) {
    std::set<Snippet*> snippets = SnippetRegistry::Instance()->Resolve(*it);
    m_snippets.insert(snippets.begin(), snippets.end());
  }
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
  std::set<std::string> all_files;
  std::map<std::string, std::vector<Snippet*> > file_to_snippet_map;
  for (auto it=all.begin();it!=all.end();it++) {
    std::string filename = (*it)->GetFileName();
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
    else {
    }
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
SPU::getContext() {
  std::string context = m_context.GetText();
  boost::regex return_regex("\\breturn\\b[^;]*;");
  context = boost::regex_replace<boost::regex_traits<char>, char>(context, return_regex, ";//replaced return\n");
  // FIXME when doing context search, break may appear, while we don't have the outside loop
  return context;
}

std::string
SPU::getInputCode() {
  std::string s;
  for (Variable var : m_inv.Variables()) {
    s += get_input_code(var);
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
SPU::GetMain() {
  std::string s;
  s += getHeader();
  s += "int main() {\n";
  s += "// Input\n";
  s += getInputCode();
  s += "// Context\n";
  s += "// " + m_filename + "\n";
  // s += m_context.GetText();
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
SPU::GetSupport() {
  // prepare the containers
  std::set<Snippet*> all_snippets;
  all_snippets = SnippetRegistry::Instance()->GetAllDependence(m_snippets);
  // simplify code: break when snippet number larger than config
  // FIXME max_snippet_number is negative?
  if (all_snippets.size() > (size_t)Config::Instance()->GetInt("max_snippet_number")) {
    m_can_continue = false;
  }
  // simplify code: break when snippet size larger than config
  int _loc = 0;
  for (auto it=all_snippets.begin();it!=all_snippets.end();it++) {
    _loc += (*it)->GetLOC();
  }
  if (_loc > Config::Instance()->GetInt("max_snippet_size")) {
    m_can_continue = false;
  }
  // sort the snippets
  std::vector<Snippet*> sorted_all_snippets = sortSnippets(all_snippets);
  // return the snippet code
  std::string code = "";
  // head
  code += get_head();
  code += SystemResolver::Instance()->GetHeaders();
  // snippets
  std::string code_func_decl;
  std::string code_func;
  for (auto it=sorted_all_snippets.begin();it!=sorted_all_snippets.end();it++) {
    if ((*it)->Type() == SK_Function) {
      // functions
      code_func_decl += dynamic_cast<FunctionSnippet*>(*it)->GetDecl()+"\n";
      code_func +=
      "// " + (*it)->GetFileName() + ":" + std::to_string((*it)->GetLineNumber())
      + "\n" + (*it)->GetCode() + '\n';
    } else {
      // all other codes
      code +=
      "// " + (*it)->GetFileName() + ":" + std::to_string((*it)->GetLineNumber())
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
SPU::GetMakefile() {
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
