#include "segment.h"
#include "utils.h"
#include <iostream>
#include "resolver.h"

#include "config.h"

#include <gtest/gtest.h>

using namespace ast;

/*******************************
 ** Construction
 *******************************/
Segment::Segment () {}
Segment::~Segment () {}

void Segment::PushBack(Node node) {
  m_nodes.push_back(node);
  m_context = m_nodes;
}
void Segment::PushBack(NodeList nodes) {
  m_nodes.insert(m_nodes.end(), nodes.begin(), nodes.end());
  m_context = m_nodes;
}
void Segment::PushFront(Node node) {
  m_nodes.insert(m_nodes.begin(), node);
  m_context = m_nodes;
}

void Segment::PushFront(NodeList nodes) {
  m_nodes.insert(m_nodes.begin(), nodes.begin(), nodes.end());
  m_context = m_nodes;
}

void Segment::Clear() {
  m_nodes.clear();
  m_context.clear();
}

/*******************************
 ** Meta info
 *******************************/


NodeList Segment::GetNodes() const {
  return m_nodes;
}

Node
Segment::GetFirstNode() const {
  if (m_nodes.empty()) {
    // this should be a node_null
    return Node();
  } else {
    return m_nodes[0];
  }
}

std::string
Segment::GetText() const {
  std::string s;
  for (Node node : m_nodes) {
    s += get_text(node);
    s += '\n'; // FIXME need this?
  }
  return s;
}

std::string
Segment::GetTextExceptComment() {
  std::string s;
  for (Node node : m_nodes) {
    s += get_text_except(node, NK_Comment);
  }
  return s;
}

// int GetLOC() const {
//   std::string text = GetText();
//   return std::count(text.begin(), text.end(), '\n');
// }


int Segment::GetLineNumber() const {
  for (Node n : m_nodes) {
    // FIXME 0 is a magic number! The acutal value inside Node is -1 ..
    if (get_node_line(n) > 0) return get_node_line(n);
  }
  return 0;
}


bool
Segment::HasNode(Node node) const {
  return ast::contains(m_nodes, node);
}

/*******************************
 * context search
 *******************************/

bool Segment::Grow() {
  // this is an empty segment, invalid
  if (m_nodes.empty()) {
    return false;
  }
  // from first node
  Node first_node = m_nodes[0];
  Node n = helium_previous_sibling(first_node);
  // has previous sibling
  if (n) {
    this->PushFront(n);
    return true;
  }
  n = helium_parent(first_node);
  // don't have parent
  if (!n) {
    return false;
  }
  // parent is function
  if (kind(n) == NK_Function) {
    m_function_nodes.push_back(n);
    NodeList calls = ast::find_nodes_from_root(n, NK_Call);
    Node call_node;
    for (Node call : calls) {
      if (call_get_name(call) == function_get_name(n)) {
        call_node =call;
        break;
      }
    }
    if (call_node) n = call_node;
    else {
      return false;
    }
  }
  this->Clear();
  this->PushFront(n);
  return true;
}


void Segment::IncreaseContext() {
  // after increasing context, should check if the segment is valid
  // 1. segment size limit
  // 2. context search value
  // // check segment size
  // std::string content = m_segment.GetText();
  // int size = std::count(content.begin(), content.end(), '\n');
  // if (size > Config::Instance()->GetInt("max_segment_size")) {
  //   return false;
  // }
  if (!Grow()) m_context_search_failed = true;
  m_context_search_time ++;
}

/**
 * Check validity status of this segment.
 * Set m_invalid_reason.
 */
bool Segment::IsValid() {
  // this is an empty segment, invalid
  if (m_nodes.empty()) {
    return false;
  }
  // check context search value
  if (m_context_search_time > Config::Instance()->GetInt("context-search-value")) {
    m_invalid_reason = "context search times larger than context-search-value limit.";
    return false;
  }
  // check segment size
  std::string text = GetText();
  int loc = std::count(text.begin(), text.end(), '\n');
  if (loc > Config::Instance()->GetInt("max-segment-size")) {
    m_invalid_reason = "size of segment larger than max-segment-size limit.";
    return false;
  }
  if (m_context_search_failed) {
    m_invalid_reason = "context search failed.";
    return false;
  }

  // check snippet limit
  // prepare the containers
  std::set<Snippet*> all_snippets;
  all_snippets = SnippetRegistry::Instance()->GetAllDeps(m_snippets);
  // simplify code: break when snippet number larger than config
  // FIXME max_snippet_number is negative?
  if (all_snippets.size() > (size_t)Config::Instance()->GetInt("max-snippet-number")) {
    m_invalid_reason = "dependent snippets larger than max-snippet-number limit.";
    return false;
  }
  // simplify code: break when snippet size larger than config
  int _loc = 0;
  for (auto it=all_snippets.begin();it!=all_snippets.end();it++) {
    _loc += (*it)->GetLOC();
  }
  if (_loc > Config::Instance()->GetInt("max-snippet-size")) {
    m_invalid_reason = "snippet size larger than max-snippet-size limit.";
    return false;
  }

  m_invalid_reason = "";
  return true;
}

/*******************************
 ** Code output
 *******************************/
std::string
Segment::getContext() {
  std::string context = ast::get_text(m_context);
  boost::regex return_regex("\\breturn\\b[^;]*;");
  context = boost::regex_replace<boost::regex_traits<char>, char>(context, return_regex, ";//replaced return\n");
  // FIXME when doing context search, break may appear, while we don't have the outside loop
  return context;
}

std::string
Segment::getInputCode() {
  std::string s;
  for (Variable var : m_inv) {
    s += get_input_code(var);
  }
  return s;
}

std::string
Segment::getHeader() {
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

std::string Segment::GetMain() {
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
get_foot() {
  return std::string()
  + "\n#endif\n";
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

std::string Segment::GetSupport() {
  // prepare the containers
  std::set<Snippet*> all_snippets;
  all_snippets = SnippetRegistry::Instance()->GetAllDeps(m_snippets);
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
    // FIXME the type of a snippet is signature, containing multiple SnippetKind
    // if ((*it)->Type() == SK_Function) {
    //   // functions
    //   code_func_decl += dynamic_cast<FunctionSnippet*>(*it)->GetDecl()+"\n";
    //   code_func +=
    //   "// " + (*it)->GetFileName() + ":" + std::to_string((*it)->GetLineNumber())
    //   + "\n" + (*it)->GetCode() + '\n';
    // } else {
    //   // all other codes
    //   code +=
    //   "// " + (*it)->GetFileName() + ":" + std::to_string((*it)->GetLineNumber())
    //   + "\n" + (*it)->GetCode() + '\n';
    // }
  }
  code += "\n// function declarations\n";
  code += code_func_decl;
  code += "\n// functions\n";
  code += code_func;
  // foot
  code += get_foot();
  return code;
}
std::string Segment::GetMakefile() {
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

/*******************************
 ** Variables
 *******************************/

void Segment::ResolveInput() {
  m_inv.clear();
  resolver::get_undefined_vars(m_context, m_inv);
}

void Segment::ResolveOutput() {
}

/*******************************
 ** Resolving
 *******************************/

std::set<std::string>
get_to_resolve(
               NodeList nodes,
               std::set<std::string> known_to_resolve,
               std::set<std::string> known_not_resolve
               ) {
  std::set<std::string> result;
  std::set<std::string> var_ids = get_var_ids(nodes);
  result.insert(var_ids.begin(), var_ids.end());
  // var_ids
  // general types in the nodes
  std::set<std::string> type_ids = get_type_ids(nodes);
  result.insert(type_ids.begin(), type_ids.end());
  // call to functions
  std::set<std::string> call_ids = get_call_ids(nodes);
  result.insert(call_ids.begin(), call_ids.end());
  // constructing
  result.insert(known_to_resolve.begin(), known_to_resolve.end());
  for (const std::string &s : known_not_resolve) {
    result.erase(s);
  }
  for (const std::string &s : c_common_keywords) {
    result.erase(s);
  }
  return result;
}


void Segment::ResolveSnippets() {
  // getting the initial set to resolve
  std::set<std::string> known_not_resolve;
  std::set<std::string> known_to_resolve;
  for (const Variable v : m_inv) {
    // the IO variable names are not needed to resolve
    known_not_resolve.insert(v.Name());
    // The type name need
    known_to_resolve.insert(v.GetType().SimpleName());
  }
  std::set<std::string> ids = get_to_resolve(m_context, known_to_resolve, known_not_resolve);
  
  m_snippets.clear();
  // the initial code to resolve is: context + input variable(input code)
  // std::string code = m_context->GetText();
  // Now we assume the comments exist in the code (no longer do the remove comment preprocessing)
  // We should not resolve the words in comments as identifiers
  // std::string code = ast::get_text_except(m_context, NK_Comment);
  // code += getInputCode();
  // TODO use semantic when resolving
  // std::set<std::string> ids = extract_id_to_resolve(code);
  for (const std::string &id : ids) {
    // std::cout <<"id: "<<id  << "\n";
    std::set<Snippet*> snippets = SnippetRegistry::Instance()->Resolve(id);
    m_snippets.insert(snippets.begin(), snippets.end());
  }
  //std::cout << SnippetRegistry::Instance()->ToString() <<"\0";
  // utils::print(SnippetRegistry::Instance()->ToString(), utils::CK_Blue);
}




/*******************************
 ** Deprecated
 *******************************/

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


// void SPU::resolveOutput() {
//   // std::set<Variable> vv;
//   // TODO point of interest
//   // uninstrument();
//   // instrument();
//   m_outv.Clear();
//   if (m_output_node) {
//     resolver::get_alive_vars(m_output_node, m_context.GetNodes(), m_outv);
//     // if (Config::Instance()->WillSimplifyOutputVar()) {
//     //   // remove already used vars in the segment/context
//     //   // m_context->GetNodes();
//     //   std::string context = getContext();
//     //   remove_used_vars(context, m_outv);
//     // }
//     /* TODO output instrument */
//     // for (auto it=m_outv.begin();it!=m_outv.end();it++) {
//     //   Node node = m_output_node.append_child("outv");
//     //   node.append_child(pugi::node_pcdata).set_value((*it)->GetOutputCode().c_str());
//     // }
//   }
// }

