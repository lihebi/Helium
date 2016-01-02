#ifndef __SNIPPET_H__
#define __SNIPPET_H__

#include <string>
#include <vector>
#include <set>
#include <map>
#include <assert.h>
#include <readtags.h>

/*******************************
 ** ctags
 *******************************/

#if 0
// this is the struct of tagEntry
typedef struct {
  /* name of tag */
  const char *name;
  /* path of source file containing definition of tag */
  const char *file;
  /* address for locating tag in source file */
  struct {
    /* pattern for locating source line
     * (may be NULL if not present) */
    const char *pattern;
    /* line number in source file of tag definition
     * (may be zero if not known) */
    unsigned long lineNumber;
  } address;
  /* kind of tag (may by name, character, or NULL if not known) */
  const char *kind;
  /* is tag of file-limited scope? */
  short fileScope;
  /* miscellaneous extension fields */
  struct {
    /* number of entries in `list' */
    unsigned short count;
    /* list of key value pairs */
    tagExtensionField *list;
  } fields;
} tagEntry;
#endif

// we need: 1. file path 2. line number 3. type
class CtagsEntry {
public:
  CtagsEntry(const tagEntry* const entry);
  ~CtagsEntry() {}
  std::string GetName() const {return m_name;}
  std::string GetFileName() const {return m_file;}
  std::string GetSimpleFileName() const { return m_simple_filename;}
  int         GetLineNumber() const {return m_line;}
  std::string GetPattern() const {return m_pattern;}
  char        GetType() const {return m_type;}
private:
  std::string m_name;           // name in index of tagfile
  std::string m_file;           // full path
  std::string m_simple_filename; // the last portion of path. Use for header sorter
  // the pattern is not used for find code.
  // The pattern itself is used in system resolver to recursively resolve type to primitive
  std::string m_pattern;
  int m_line;
  char m_type; // one char type
};



void ctags_init(const std::string& filename);
std::vector<CtagsEntry> ctags_parse(const std::string& name);


/*******************************
 ** ctags type enum and help functions
 *******************************/
/*
 * Snippet Type
 *  f: function
 *  s: structure
 *  g: enumerators
 *  u: union
 *  d: define
 *  v: variabl
 *  e: enumerator values
 *  t: typedef
 *  c: constant/classes
 *  m: class/struct/union members
 */

enum ctags_type {
  CTAGS_FUNC = 1,        // f: function
  CTAGS_STRUCT,          // s: structure
  CTAGS_ENUM,            // g: enumerator
  CTAGS_UNION,           // u: union
  CTAGS_DEF,             // d: define
  CTAGS_VAR,             // v: variable
  CTAGS_ENUM_MEM,        // e: enumerator member
  CTAGS_TYPEDEF,         // t: typedef
  CTAGS_CONST,           // c: constant
  CTAGS_MEM              // m: class/struct/union member // not useful
};

enum ctags_type
char_to_ctags_type(char t);
std::set<enum ctags_type>
string_to_ctags_types(std::string s);
char
ctags_type_to_char(enum ctags_type t);
std::string
ctags_types_to_string(std::set<enum ctags_type> types);

/*******************************
 ** Functions for get code from file based on ctags entry
 *******************************/
std::string get_func_code(const CtagsEntry& entry);
std::string get_enum_code(const CtagsEntry& entry);
std::string get_def_code(const CtagsEntry& entry);
std::string get_struct_code(const CtagsEntry& entry);
std::string get_union_code(const CtagsEntry& entry);
std::string get_var_code(const CtagsEntry& entry);
std::string get_typedef_code(const CtagsEntry& entry);


/*******************************
 ** Snippet
 *******************************/
typedef std::multimap<std::string, enum ctags_type> snippet_signature;

class Snippet {
public:
  Snippet(const CtagsEntry& entry);
  ~Snippet();
  std::string GetName();

  snippet_signature GetSignature();
  std::set<enum ctags_type> GetSignature(const std::string& name);
  bool SatisfySignature(const std::string& name, std::set<enum ctags_type> types);

  std::string GetCode();
private:
  snippet_signature m_sig;
  std::string m_code;
};


/*
 * Check if an identifier is a system function or type.
 */

class SystemResolver {
public:
  static SystemResolver* Instance() {
    if (m_instance == 0) {
      m_instance = new SystemResolver();
    }
    return m_instance;
  }
  // load the systype.tags file
  void Load(const std::string& filename);
  // resolve to primitive type
  std::string ResolveType(const std::string& type);
  std::vector<CtagsEntry> Parse(const std::string& name) ;
  std::vector<CtagsEntry> Parse(const std::string& name, const std::string& type);
  bool Has(const std::string& name);
  std::string GetHeaders() const;
  std::string GetLibs() const;
private:
  SystemResolver();
  ~SystemResolver() {}
  // std::vector<Header> m_headers; // header files used
  static SystemResolver* m_instance;
  tagFile *m_tagfile;
  tagEntry *m_entry;
  // headers that need to be included
  std::set<std::string> m_headers;
  std::set<std::string> m_libs; // library compilation flags
};





/**
 * Registry for storing all resolved snippets, exactly once per ID.

Design of snippet registry
==========================

Lookup
------
client can lookup snippet by their name and type.

| type | name | alias | code |
| stucture | name | alias | code |
| enum | name | alias | code | member |

some name can have multiple entries.

e.g. typedef struct conn* conn;
struct conn conn;

here conn is a structure name, a typedef, as well as a variable.

Adding
------
When adding a type, we need to resolve recursively the dependencies of this snippet.
Also, we need to record this dependency, so that when we know a snippet is required by a segment, we can extract all snippets that needs to build the segment.

Algorithm goes here:

Input:
* id to resolve
* [optional] type of id
Output: a list of snippet of id, possibly with the correct type.
Side effect: even if user specified the type to resolve, we will resolve all snippets that use it.

However, the snippets of the same ID may depend on each other.
E.g. typedef struct conn* conn;
"typedef conn" will depend on "struct conn".

Also, the ID may not be able to split:
typedef struct conn { ... } * conn;

 */
class SnippetRegistry {
public:
  static SnippetRegistry* Instance() {
    if (m_instance == 0) {
      m_instance = new SnippetRegistry();
    }
    return m_instance;
  }

  /**
   * Resolve the id "name", return snippet if found, or empty set if cannot resolve.
   Internally, it resolve the id, for all types, recursively.
   Every time the lookup hit the record, that means for this entry, nothing needs to be done.
   We resolve everything at once.
   * This is the only API to add something into registry from outside.
   * Ctags resolver will not be directly used by client.
   */
  std::set<Snippet*> Resolve(const std::string& name, std::string type="");
  
  std::set<Snippet*> GetDependence(Snippet* snippet);
  // recursively get dependence
  std::set<Snippet*> GetAllDependence(Snippet* snippet);
  std::set<Snippet*> GetAllDependence(std::set<Snippet*> snippets);

private:
  std::set<Snippet*> lookUp(const std::string& name);
  // look up by type
  Snippet* lookUp(const std::string& name, char type);
  // look up by types
  std::set<Snippet*> lookUp(const std::string& name, const std::string& type);
  
  Snippet* createSnippet(const CtagsEntry& ce);
  // Can not add dependence outside the class
  void addDependence(Snippet *from, Snippet *to);
  void addDependence(Snippet *from, std::set<Snippet*> to);
  void resolveDependence(Snippet *s, int level);
  void add(Snippet *s);
  SnippetRegistry() {}
  // resolve dependence
  static SnippetRegistry* m_instance;
  // this is where actually store the resolved code snippets
  std::set<Snippet*> m_snippets;
  /*
   * id is the simplest form, without any keywords
   * keywords should be in the type field of the snippet
   */
  std::map<std::string, std::set<Snippet*> > m_id_map;
  /*
   * since every snippet is created and allocated in SnippetRegistry,
   * the address of the pointers should be unique
   * so we map from pointer to pointer vector, representing the dependence
   */
  std::map<Snippet*, std::set<Snippet*> > m_dependence_map;
};

#endif
