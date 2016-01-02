#include <pugixml.hpp>
#include <iostream>
#include "snippet.h"
#include "utils.h"

/*******************************
 ** ctags
 *******************************/

CtagsEntry::CtagsEntry(const tagEntry* const entry) {
  m_name = entry->name;
  m_file = entry->file;
  m_line = entry->address.lineNumber;
  m_pattern = entry->address.pattern;
  m_type = *(entry->kind);
  if (m_file.find("/") != std::string::npos) {
    m_simple_filename = m_file.substr(m_file.rfind("/")+1);
  }
}

static tagFile *tag_file;
/**
 * Init tag file.
 * If tag_file is not NULL, free it, and then 
 * @param[in] filename
 * @side modify static variable tag_file, which is the ctags handle.
 * Must be called before ctags_parse.
 */
void ctags_load(const std::string& filename) {
  if (tag_file != NULL) tagsClose(tag_file);
  tagFileInfo *info = (tagFileInfo*)malloc(sizeof(tagFileInfo));
  tag_file = tagsOpen(filename.c_str(), info);
  if (info->status.opened != true) {
    assert(false);
  }
  free(info);
}

/**
 * Parse name in tag file.
 * @return a vector of CtagsEntry
 */
std::vector<CtagsEntry> ctags_parse(const std::string& name) {
  assert(tag_file != NULL);
  tagEntry *tag_entry = (tagEntry*)malloc(sizeof(tagEntry));
  std::vector<CtagsEntry> entries;
  tagResult result = tagsFind(tag_file, tag_entry, name.c_str(), TAG_FULLMATCH);
  while (result == TagSuccess) {
    if (tag_entry->kind) {
      entries.push_back(CtagsEntry(tag_entry));
    }
    result = tagsFindNext(tag_file, tag_entry);
  }
  return entries;
}
  


/*******************************
 ** ctags_type enum related
 *******************************/

enum ctags_type char_to_ctags_type(char t) {
  switch (t) {
  case 'f': return CTAGS_FUNC;
  case 's': return CTAGS_STRUCT;
  case 'g': return CTAGS_ENUM;
  case 'u': return CTAGS_UNION;
  case 'd': return CTAGS_DEF;
  case 'v': return CTAGS_VAR;
  case 'e': return CTAGS_ENUM_MEM;
  case 't': return CTAGS_TYPEDEF;
  case 'c': return CTAGS_CONST;
  case 'm': return CTAGS_MEM;
  default: assert(false);
  }
}

std::set<enum ctags_type> string_to_ctags_types(std::string s) {
  std::set<enum ctags_type> types;
  for (auto it=s.begin();it!=s.end();it++) {
    types.insert(char_to_ctags_type(*it));
  }
  return types;
}

char ctags_type_to_char(enum ctags_type t) {
  switch (t) {
  case CTAGS_FUNC : return 'f';
  case CTAGS_STRUCT : return 's';
  case CTAGS_ENUM : return 'g';
  case CTAGS_UNION : return 'u';
  case CTAGS_DEF : return 'd';
  case CTAGS_VAR : return 'v';
  case CTAGS_ENUM_MEM : return 'e';
  case CTAGS_TYPEDEF : return 't';
  case CTAGS_CONST : return 'c';
  case CTAGS_MEM : return 'm';
  default: assert(false);
  }
}

std::string ctags_types_to_string(std::set<enum ctags_type> types) {
  std::string s;
  for (auto it=types.begin();it!=types.end();it++) {
    s += ctags_type_to_char(*it);
  }
  return s;
}

/*******************************
 ** Snippet
 *******************************/

/**
 * Get all types pairs of this snippet.

 This should be a multimap
 {
 "conn": CTAGS_STRUCT,
 "conn": CTAGS_TYPEDEF
 }

 {
 "ctags_type": CTAGS_ENUM,
 "CTAGS_CONST": CTAGS_ENUM_MEM,
 "CTAGS_MEM": CTAGS_ENUM_MEM
 }

 The snippet will only be indexed by the name, e.g. "conn".

 typedef struct {
 } ALIAS_NAME;

 Then the signature is {"ALIAS_NAME", CTAGS_TYPEDEF} ? but actually it is a structure.

 每个模块只专心做一件事，所以这个不必理会，直接按照typedef来说。
 因为大体上用到snippet只是要知道其code，以及能够管理dependency和lookup。
 只有Type init时才会考虑到其member，只有在其是函数时才会考虑到其declaration。
 因此，写一些help函数来判断一个typedf是什么，就行了。

*/
snippet_signature
Snippet::GetSignature() {
  return m_sig;
}
/**
 * @param[in] name only get the types of the id(key) "name"
 */
std::set<enum ctags_type>
Snippet::GetSignature(const std::string& name) {
  std::pair <snippet_signature::iterator, snippet_signature::iterator> ret; 
  std::set<enum ctags_type> types;
  ret = m_sig.equal_range(name);
  for (auto it=ret.first; it!=ret.second;it++) {
    types.insert(it->second);
  }
  return types;
}

/**
 * True if this snippet has a key "name", whose type has a map to ONE of "types"
 */
bool
Snippet::SatisfySignature(const std::string& name, std::set<enum ctags_type> types) {
  std::pair <snippet_signature::iterator, snippet_signature::iterator> ret;
  ret = m_sig.equal_range(name);
  for (auto it=ret.first; it!=ret.second;it++) {
    if (types.find(it->second) != types.end()) {
      return true;
    }
  }
  return false;
}

Snippet::~Snippet() {}
// used only for print purpose! Human readable.
std::string Snippet::GetName() {
  return "NO NAME";
}






/*******************************
 ** Functions for get code from file based on ctags entry
 *******************************/

/*
 * use depth-first-search for the first pos:line attribute
 * return -1 if no pos:line attr found
 */
int
get_element_line(pugi::xml_node node) {
  // check if pos:line is enabled on this xml
  pugi::xml_node root = node.root();
  if (!root.child("unit").attribute("xmlns:pos")) {
    std::cerr<<"position is not enabled in srcml"<<std::endl;
    exit(1);
    return -1;
  }
  // the node itself has pos:line attr, just use it
  if (node.attribute("pos:line")) {
    return atoi(node.attribute("pos:line").value());
  } else {
    pugi::xml_node n = node.select_node("//*[@pos:line]").node();
    if (n) {
      return atoi(n.attribute("pos:line").value());
    }
  }
  return -1;
}

/*
 * The last pos:line in the current node element
 * Useful to track the range of the current AST node.
 */
int
get_element_last_line(pugi::xml_node node) {
  pugi::xml_node root = node.root();
  if (!root.child("unit").attribute("xmlns:pos")) {
    std::cerr<<"position is not enabled in srcml"<<std::endl;
    exit(1);
    return -1;
  }
  pugi::xml_node n = node.select_node("(//*[@pos:line])[last()]").node();
  
  // pugi::xpath_node_set nodes = node.select_nodes("//*[@pos:line]");
  // pugi::xml_node last_node = nodes[nodes.size()-1].node();
  // return atoi(last_node.attribute("pos:line").value());
  
  if (n) {
    return atoi(n.attribute("pos:line").value());
  } else if (node.attribute("pos:line")) {
    return atoi(node.attribute("pos:line").value());
  }
  return -1;
}

/**
 * Get code as string of <tag_name> node in filename that encloses line_number
 */
std::string get_code_enclosing_line(const std::string& filename, int line_number, std::string tag_name) {
  pugi::xml_document doc;
  file2xml(filename, doc);
  pugi::xml_node root =doc.document_element();
  std::string query = "//" + tag_name;
  pugi::xpath_node_set nodes = root.select_nodes(query.c_str());
  for (auto it=nodes.begin();it!=nodes.end();it++) {
    pugi::xml_node node = it->node();
    int first_line = get_element_line(node);
    int last_line = get_element_last_line(node);
    // FIXME the equal is necessary? Be precise.
    if (first_line <= line_number && last_line >= line_number) {
      return get_text_content(node);
    }
  }
  return "";
}

/**
 * Use filename and line number to match a <funciton> that contains that line.
 */
std::string get_func_code(const CtagsEntry& entry) {
  int line_number = entry.GetLineNumber();
  std::string filename = entry.GetFileName();
  return get_code_enclosing_line(filename, line_number, "function");
}

/**
 * Use filename and line number to match a <enum> that contains the line.
 * FIXME the enum member may be of a anonymouse enum, within another struct.
 * FIXME The line number containing technique should be tested.
 */
std::string get_enum_code(const CtagsEntry& entry) {
  int line_number = entry.GetLineNumber();
  std::string filename = entry.GetFileName();
  std::string enum_code = get_code_enclosing_line(filename, line_number, "enum");
  std::string typedef_code = get_code_enclosing_line(filename, line_number, "typedef");
  if (!typedef_code.empty()) {
    return typedef_code;
  } else {
    return enum_code;
  }

}


std::string get_def_code(const CtagsEntry& entry) {
                            // std::string filename, int line) {
  int line_number = entry.GetLineNumber();
  std::string filename = entry.GetFileName();
  return get_code_enclosing_line(filename, line_number, "cpp:define");
}

std::string get_typedef_code(const CtagsEntry& entry) {
  int line_number = entry.GetLineNumber();
  std::string filename = entry.GetFileName();
  return get_code_enclosing_line(filename, line_number, "typedef");
}

/**
 * If the struct is a typedef, I should not just get the <struct> tag, but the <typedef> tag.
 * Because 1) only get the struct is not syntax-valid(miss ;);
 * and 2) the typedef ID will need another snippet, which will cause the same code appears in different places.
 * 
 * The baseline of Snippet class is that, the same code can NEVER be in different places(Snippets).
 */
std::string get_struct_code(const CtagsEntry& entry) {
  int line_number = entry.GetLineNumber();
  std::string filename = entry.GetFileName();
  std::string struct_code =  get_code_enclosing_line(filename, line_number, "struct");
  std::string typedef_code = get_code_enclosing_line(filename, line_number, "typedef");
  /*
    If there's also a <typedef> enclosing this line, than we use it,
    because we don't want same code in different Snippets.
  */
  if (!typedef_code.empty()) {
    return typedef_code;
  } else {
    return struct_code;
  }
}

/**
 * FIXME same as above
 */
std::string get_union_code(const CtagsEntry& entry) {
  int line_number = entry.GetLineNumber();
  std::string filename = entry.GetFileName();
  std::string union_code =  get_code_enclosing_line(filename, line_number, "union");
  std::string typedef_code = get_code_enclosing_line(filename, line_number, "typedef");
  if (!typedef_code.empty()) {
    return typedef_code;
  } else {
    return union_code;
  }
}

/**
 * FIXME the decl_stmt that enclose the line may contains other variable!!
 */
std::string get_var_code(const CtagsEntry& entry) {
  int line_number = entry.GetLineNumber();
  std::string filename = entry.GetFileName();
  return get_code_enclosing_line(filename, line_number, "decl_stmt");
}

/*******************************
 ** create snippet
 *******************************/

std::vector<std::string> get_enum_members(const std::string &code) {
  std::vector<std::string> members;
  pugi::xml_document doc;
  string2xml(code, doc);
  pugi::xml_node root_node = doc.document_element();
  pugi::xml_node enum_node = root_node.select_node("//enum").node();
  pugi::xpath_node_set name_nodes = enum_node.select_nodes("block/decl/name");
  for (size_t i=0;i<name_nodes.size();i++) {
    std::string s = get_text_content(name_nodes[i].node());
    // Add enum member names into keywords
    if (!s.empty()) {
      members.push_back(s);
    }
  }
  return members;
}

/**
 * Query query on code.
 * return the first matching.
 */
std::string query_code_first(const std::string& code, const std::string& query) {
  pugi::xml_document doc;
  string2xml(code, doc);
  pugi::xml_node root_node = doc.document_element();
  pugi::xml_node node = root_node.select_node(query.c_str()).node();
  return node.child_value();
}
/**
 * Query "query" on "code".
 * Return all matching.
 * Will not use get_text_content, but use child_value() for a xml tag.
 * Only support tag value currently, not attribute value.
 */
std::vector<std::string> query_code(const std::string& code, const std::string& query) {
  std::vector<std::string> result;
  pugi::xml_document doc;
  string2xml(code, doc);
  pugi::xml_node root_node = doc.document_element();
  pugi::xpath_node_set nodes = root_node.select_nodes(query.c_str());
  for (auto it=nodes.begin();it!=nodes.end();it++) {
    pugi::xml_node node = it->node();
    result.push_back(node.child_value());
  }
  return result;
}

Snippet::Snippet(const CtagsEntry& entry) {
  /**
   * 1. get code
   * 2. get signature
   */
  enum ctags_type type = char_to_ctags_type(entry.GetType());
  switch(type) {
  case CTAGS_FUNC: {
    m_code = get_func_code(entry);
    m_sig.emplace(entry.GetName(), CTAGS_FUNC);
    break;
  }
  case CTAGS_STRUCT: {
    m_code = get_struct_code(entry);
    m_sig.emplace(entry.GetName(), CTAGS_STRUCT);
    break;
  }
  case CTAGS_ENUM: {
    m_code = get_enum_code(entry);
    m_sig.emplace(entry.GetName(), CTAGS_ENUM);
    // std::vector<std::string> members = get_enum_members(m_code);
    // FIXME TEST this!!! HEBI can I just use this, without the detailed "block"?
    std::vector<std::string> members = query_code(m_code, "//enum/decl/name");
    for (std::string m : members) {
      m_sig.emplace(m, CTAGS_ENUM_MEM);
    }
    break;
  }
  case CTAGS_UNION: {
    m_code = get_union_code(entry);
    m_sig.emplace(entry.GetName(), CTAGS_UNION);
    break;
  }
  case CTAGS_DEF: {
    m_code = get_def_code(entry);
    m_sig.emplace(entry.GetName(), CTAGS_DEF);
    break;
  }
  case CTAGS_VAR: {
    m_code = get_var_code(entry);
    m_sig.emplace(entry.GetName(), CTAGS_VAR);
    break;
  }
  case CTAGS_ENUM_MEM: {
    m_code = get_enum_code(entry);
    // std::vector<std::string> members = get_enum_members(m_code);
    std::vector<std::string> members = query_code(m_code, "//enum/decl/name");
    for (std::string m : members) {
      m_sig.emplace(m, CTAGS_ENUM_MEM);
    }
    // enum name
    std::string name = query_code_first(m_code, "//enum/name");
    if (!name.empty()) m_sig.emplace(name, CTAGS_ENUM);
    // possibly typedef
    name = query_code_first(m_code, "//typedef/name");
    if (!name.empty()) m_sig.emplace(name, CTAGS_TYPEDEF);
    break;
  }
  case CTAGS_TYPEDEF: {
    m_code = get_typedef_code(entry);
    // tyepdef
    std::string name = query_code_first(m_code, "//typedef/name");
    if (!name.empty()) m_sig.emplace(name, CTAGS_TYPEDEF);
    // TODO NOW also needs to test if it is a struct, union, or enum
    break;
  }
  // case CTAGS_CONST:
  //   m_code = get_const_code(entry);
  // case CTAGS_MEM:
  //   m_code = get_mem_code(entry);
  default:
    // should we reach here?
    // null snippet?
    m_code = "";
  }
}
