#include <pugixml.hpp>
#include <iostream>
#include "helium/resolver/snippet.h"
#include "helium/utils/log.h"
#include "helium/utils/utils.h"

#include "helium/parser/xmlnode.h"
#include "helium/parser/xmlnode_helper.h"
#include "helium/resolver/system_resolver.h"

#include "helium/parser/xml_doc_reader.h"


using namespace utils;




/*******************************
 ** ctags
 *******************************/

CtagsEntry::CtagsEntry(const tagEntry* const entry) {
  // print_trace("CtagsEntry::CtagsEntry");
  m_name = entry->name;
  m_file = entry->file;
  m_line = entry->address.lineNumber;
  m_pattern = entry->address.pattern;
  if (entry->kind) {
    m_type = *(entry->kind);
  } else {
    helium_print_warning("Warning: CtagsEntry::CtagsEntry: entry->kind == NULL");
    // std::cout << "warning:"  << "\n";
    // std::cout << m_name  << "\n";
    // std::cout << m_file  << "\n";
    // std::cout << m_line  << "\n";
    // std::cout << m_pattern  << "\n";
    m_type = 'x';
    // assert(false);
  }
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

SnippetKind char_to_snippet_kind(char t) {
  switch (t) {
  case 'f': return SK_Function;
  case 's': return SK_Structure;
  case 'g': return SK_Enum;
  case 'u': return SK_Union;
  case 'd': return SK_Define;
  case 'v': return SK_Variable;
  case 'e': return SK_EnumMember;
  case 't': return SK_Typedef;
  case 'c': return SK_Const;
  case 'm': return SK_Member;
  case 'x': return SK_Other;
  default: {
    std::cerr <<t  << "\n";
    assert(false);
  }
  }
}

std::set<SnippetKind> string_to_snippet_kinds(std::string s) {
  std::set<SnippetKind> types;
  for (auto it=s.begin();it!=s.end();it++) {
    types.insert(char_to_snippet_kind(*it));
  }
  return types;
}

char snippet_kind_to_char(SnippetKind t) {
  switch (t) {
  case SK_Function : return 'f';
  case SK_Structure : return 's';
  case SK_Enum : return 'g';
  case SK_Union : return 'u';
  case SK_Define : return 'd';
  case SK_Variable : return 'v';
  case SK_EnumMember : return 'e';
  case SK_Typedef : return 't';
  case SK_Const : return 'c';
  case SK_Member : return 'm';
  case SK_Other : return 'x';
  default: assert(false);
  }
}

std::string snippet_kinds_to_string(std::set<SnippetKind> types) {
  std::string s;
  for (auto it=types.begin();it!=types.end();it++) {
    s += snippet_kind_to_char(*it);
  }
  return s;
}

/*******************************
 ** Snippet
 *******************************/


/*******************************
 ** create snippet
 *******************************/

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

Snippet::Snippet(const CtagsEntry& entry) : m_entry(entry) {
  helium_print_trace("Snippet::Snippet");
  /**
   * 1. get code
   * 2. get signature
   */
  SnippetKind type = char_to_snippet_kind(entry.GetType());
  m_filename = entry.GetFileName();
  m_line_number = entry.GetLineNumber();
  switch(type) {
  case SK_Function: {
    m_main_kind = SK_Function;
    m_main_name = entry.GetName();
    m_code = get_func_code(entry);
    // int loc = std::count(m_code.begin(), m_code.end(), '\n');
    // if (loc == 222) {
    //   std::cout << "1064: ";
    //   std::cout << "function: ";
    //   std::cout <<m_main_name << ":";
    //   std::cout << entry.GetFileName()  << "\n";
    //   std::cout << entry.GetLineNumber()  << "\n";
    //   std::cout << loc  << "\n";
    // }
    // m_sig.emplace(entry.GetName(), SK_Function);
    m_sig[entry.GetName()].insert(SK_Function);
    break;
  }
  case SK_Structure: {
    m_main_kind = SK_Structure;
    m_main_name = entry.GetName();
    m_code = get_struct_code(entry);
    // m_sig.emplace(entry.GetName(), SK_Structure);
    m_sig[entry.GetName()].insert(SK_Structure);
    // TODO NOW may also be a typedef
    if (m_code.find("typedef") == 0) {
      m_main_kind = SK_Typedef;
      m_main_name = query_code_first(m_code, "//typedef/name");
      // // m_sig.emplace(m_main_name, SK_Typedef);
      m_sig[m_main_name].insert(SK_Typedef);
    }
    break;
  }
  case SK_Enum: {
    m_main_kind = SK_Enum;
    m_main_name = entry.GetName();
    m_code = get_enum_code(entry);
    // m_sig.emplace(entry.GetName(), SK_Enum);
    m_sig[entry.GetName()].insert(SK_Enum);
    // FIXME TEST this!!! HEBI can I just use this, without the detailed "block"?
    std::vector<std::string> members = query_code(m_code, "//enum/block/decl/name");
    for (std::string m : members) {
      // m_sig.emplace(m, SK_EnumMember);
      m_sig[m].insert(SK_EnumMember);
    }
    // TODO NOW may also be a typedef
    if (m_code.find("typedef") == 0) {
      m_main_kind = SK_Typedef;
      m_main_name = query_code_first(m_code, "//typedef/name");
      // m_sig.emplace(m_main_name, SK_Typedef);
      m_sig[m_main_name].insert(SK_Typedef);
    }

    break;
  }
  case SK_Union: {
    m_main_kind = SK_Union;
    m_main_name = entry.GetName();
    m_code = get_union_code(entry);
    // m_sig.emplace(entry.GetName(), SK_Union);
    m_sig[entry.GetName()].insert(SK_Union);
    // TODO NOW may also be a typedef
    if (m_code.find("typedef") == 0) {
      m_main_kind = SK_Typedef;
      m_main_name = query_code_first(m_code, "//typedef/name");
      // m_sig.emplace(m_main_name, SK_Typedef);
      m_sig[m_main_name].insert(SK_Typedef);
    }
    break;
  }
  case SK_Define: {
    // debug_time();
    m_main_kind = SK_Define;
    m_main_name = entry.GetName();
    // debug_time("1");
    m_code = get_def_code(entry);
    // debug_time("2");
    // m_sig.emplace(entry.GetName(), SK_Define);
    m_sig[entry.GetName()].insert(SK_Define);
    // debug_time("3");
    break;
  }
  case SK_Variable: {
    m_main_kind = SK_Variable;
    m_main_name = entry.GetName();
    m_code = get_var_code(entry);
    // FIXME this may contain variable, as well as /structure/!
    // m_sig.emplace(entry.GetName(), SK_Variable);
    m_sig[entry.GetName()].insert(SK_Variable);
    break;
  }
  case SK_EnumMember: {
    m_code = get_enum_code(entry);
    std::vector<std::string> members = query_code(m_code, "//enum/block/decl/name");
    for (std::string m : members) {
      // m_sig.emplace(m, SK_EnumMember);
      m_sig[m].insert(SK_EnumMember);
    }
    // enum name
    std::string name = query_code_first(m_code, "//enum/name");
    if (!name.empty()) {
      // m_sig.emplace(name, SK_Enum);
      m_sig[name].insert(SK_Enum);
      // set main 1
      m_main_kind = SK_Enum;
      m_main_name = name;
    }
    // possibly typedef
    name = query_code_first(m_code, "//typedef/name");
    if (!name.empty()) {
      // m_sig.emplace(name, SK_Typedef);
      m_sig[name].insert(SK_Typedef);
      // set main 2
      m_main_kind = SK_Typedef;
      m_main_name = name;
    }
    break;
  }
  case SK_Typedef: {
    m_main_kind = SK_Typedef;
    m_main_name = entry.GetName();
    m_code = get_typedef_code(entry);
    // tyepdef
    // this may be empty
    std::string name = query_code_first(m_code, "//typedef/name");
    // if (!name.empty()) m_sig.emplace(name, SK_Typedef);
    if (!name.empty()) {
      m_sig[name].insert(SK_Typedef);
    }
    // other values
    std::string inner;
    // struct
    inner = query_code_first(m_code, "//typedef/type/struct/name");
    if (!inner.empty()) {
      // m_sig.emplace(inner, SK_Structure);
      m_sig[inner].insert(SK_Structure);
    }
    // enum
    /**
     * FIXME typedef enum {aaa,bbb} xxx;
     */
    inner = query_code_first(m_code, "//typedef/type/enum/name");
    if (!inner.empty()) {
      // m_sig.emplace(inner, SK_Enum);
      m_sig[inner].insert(SK_Enum);
    }
    std::vector<std::string> members = query_code(m_code, "//enum/block/decl/name");
    for (std::string m : members) {
      // m_sig.emplace(m, SK_EnumMember);
      m_sig[m].insert(SK_EnumMember);
    }
    // union
    inner = query_code_first(m_code, "//typedef/type/union/name");
    if (!inner.empty()) {
      // m_sig.emplace(inner, SK_Union);
      m_sig[inner].insert(SK_Union);
    }
    // function pointer
    // FIXME typedef uint32_t (*hash_func)(const void *key, size_t length);
    // FIXME typedef RETSIGTYPE (*sig_type) OF((int)); => <macro>
    inner = query_code_first(m_code, "//typedef/function_decl/name");
    if (!inner.empty()) {
      // FIXME Do I need to have a seperate class for it?
      // because the SK_Typedef may have a method to get its name, which will not be consistant with this one.
      // m_sig.emplace(inner, SK_Typedef);
      m_sig[inner].insert(SK_Typedef);
    }
    break;
  }
  // case SK_Const:
  //   m_code = get_const_code(entry);
  // case SK_Member:
  //   m_code = get_mem_code(entry);
  default:
    // should we reach here?
    // yes, we should. But, we are not getting anything interesting.
    // this is because we don't care about 'm' type
    // null snippet?
    // this is SK_Const, or more likely, SK_Member
    m_code = ""; // this indicate the snippet is invalid.
    return;
  }

  // if (m_sig.empty()) {
  //   // std::cout << "WARNING: No Sig Found!"  << "\n";
  //   // FIXME should I add this for everyone?
  //   // typedef RETSIGTYPE (*sig_type) OF((int));
  //   m_sig.emplace(entry.GetName(), type);
  // }
  if (m_sig.count(entry.GetName()) == 0) {
    // m_sig.emplace(entry.GetName(), type);
    m_sig[entry.GetName()].insert(type);
  }
  m_loc = std::count(m_code.begin(), m_code.end(), '\n');
}



/**
 * Get all types pairs of this snippet.

 This should be a multimap
 {
 "conn": SK_Structure,
 "conn": SK_Typedef
 }

 {
 "ctags_type": SK_Enum,
 "SK_Const": SK_EnumMember,
 "SK_Member": SK_EnumMember
 }

 The snippet will only be indexed by the name, e.g. "conn".

 typedef struct {
 } ALIAS_NAME;

 Then the signature is {"ALIAS_NAME", SK_Typedef} ? but actually it is a structure.

 每个模块只专心做一件事，所以这个不必理会，直接按照typedef来说。
 因为大体上用到snippet只是要知道其code，以及能够管理dependency和lookup。
 只有Type init时才会考虑到其member，只有在其是函数时才会考虑到其declaration。
 因此，写一些help函数来判断一个typedf是什么，就行了。

*/
SnippetSignature
Snippet::GetSignature() const {
  return m_sig;
}
/**
 * @param[in] name only get the types of the id(key) "name"
 */
std::set<SnippetKind>
Snippet::GetSignature(const std::string& name) {
  if (m_sig.count(name) == 1) return m_sig[name];
  return {};
}

/**
 * Get all keys of m_sig. Which is all keywords to index this snippet in registry
 */
std::set<std::string> Snippet::GetSignatureKey() const {
  std::set<std::string> result;
  for (auto sig : m_sig) {
    result.insert(sig.first);
  }
  return result;
}

/**
 * True if this snippet has a key "name", whose type has a map to ONE of "types"
 */
bool
Snippet::SatisfySignature(const std::string& name, std::set<SnippetKind> types) {
  if (m_sig.count(name) == 1) {
    std::set<SnippetKind> mykinds = m_sig[name];
    for (SnippetKind k : mykinds) {
      if (types.count(k) == 1) return true;
    }
  }
  return false;
}

Snippet::~Snippet() {}
// used only for print purpose! Human readable.
std::string Snippet::GetName() const {
  return "NO NAME";
}

/**
 * For print purpose.
 */
std::string Snippet::ToString() const {
  std::string result;
  result += "{";
  for (auto m : m_sig) {
    std::string key = m.first;
    std::set<SnippetKind> kinds =m_sig.at(key);
    // result += "key: " + key + " type: " + snippet_kind_to_char(k) + "\n";
    result += key + ":";
    for (SnippetKind k : kinds) {
      result += snippet_kind_to_char(k);
      result += ", ";
    }
    // snippet_kind_to_char(k) + ", ";
  }
  result += "}";
  return result;
}



/*******************************
 ** Functions for get code from file based on ctags entry
 *******************************/


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
struct AA {
char *t;
} *a;

will be <struct>, and *a will not be a <decl_stmt>

FIXME the function pointer is also recognized as variable by ctags
int (*work) (int infile, int outfile) = zip;
int (*read_buf) (char *buf, unsigned size);
 */
std::string get_var_code(const CtagsEntry& entry) {
  int line_number = entry.GetLineNumber();
  std::string filename = entry.GetFileName();
  // return get_code_enclosing_line(filename, line_number, "decl_stmt");
  std::string ret;
  ret = get_code_enclosing_line(filename, line_number, "decl_stmt");
  if (ret.empty()) {
    // If the structure has a name, then it will be the structure, together with the variable.
    // need to ensure that will be duplicate, i.e. the signature is fully chaptured, for both struct and variable
    ret = get_code_enclosing_line(filename, line_number, "struct");
    if (!ret.empty()) return ret;
    ret = get_code_enclosing_line(filename, line_number, "enum");
    if (!ret.empty()) return ret;
    ret = get_code_enclosing_line(filename, line_number, "union");
    if (!ret.empty()) return ret;
  }
  if (ret.empty()) {
    // check if it is a function decl.
    ret = get_code_enclosing_line(filename, line_number, "function_decl");
  }
  return ret;
}





// TODO NOW Additional check
// if the snippet redefine a system level structure or function that we use, in the header files
// we do not create it here
// Because that will cause the conflict type if this snippet is used
// The reason why this may appear is that, for instance,
// the gzip-1.2.4, there's a sysStat.h file that emulate the Unix sys/stat.h for PRIMOS,
// but this file is not included in any files ... Holy shit.
bool Snippet::IsValid() const {
  if (m_code.empty()) return false;
  std::vector<CtagsEntry> entries = SystemResolver::Instance()->Parse(m_main_name);
  for (CtagsEntry entry : entries) {
    if (entry.GetName() == m_entry.GetName() && entry.GetType() == m_entry.GetType()) {
      // this is not good
      // std::cerr << "Snippet duplicate with a system one: " << entry.GetName() << " of type " << entry.GetType() << ". Skipped.\n";

      if (m_main_kind == SK_Variable) {
        // std::cerr << "BUT, IGNORED for now, because it is a variable" << "\n";
        continue;
      }
      return false;
    }
  }
  return true;
}
