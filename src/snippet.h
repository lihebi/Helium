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



void ctags_load(const std::string& filename);
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

typedef enum _SnippetKind {
  SK_Function,      // f: function                               
  SK_Structure,     // s: structure                              
  SK_Enum,          // g: enumerator                             
  SK_Union,         // u: union                                  
  SK_Define,        // d: define                                 
  SK_Variable,      // v: variable                               
  SK_EnumMember,    // e: enumerator member                      
  SK_Typedef,       // t: typedef                                
  SK_Const,         // c: constant                               
  SK_Member,         // m: class/struct/union member // not useful
  SK_Other           // x?
} SnippetKind;


SnippetKind
char_to_snippet_kind(char t);
std::set<SnippetKind>
string_to_snippet_kinds(std::string s);
char
snippet_kind_to_char(SnippetKind t);
std::string
snippet_kinds_to_string(std::set<SnippetKind> types);

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
// typedef std::multimap<std::string, SnippetKind> snippet_signature;

typedef std::map<std::string, std::set<SnippetKind> > snippet_signature;

class Snippet {
public:
  Snippet(const CtagsEntry& entry);
  virtual ~Snippet();
  std::string GetName() const;
  // TODO do we really need this, since we already has the more powerful signature?
  // Maybe a simple utility function to get coerce categorization?
  // SnippetKind Type() const;

  /* signature */
  snippet_signature GetSignature() const;
  /* snippet will be looked-up by name. */
  std::set<SnippetKind> GetSignature(const std::string& name);
  std::set<std::string> GetSignatureKey() const;
  SnippetKind MainKind() const {return m_main_kind;}
  std::string MainName() const {return m_main_name;}
  /* this is not name has all types, but name has a mapping to one of types */
  bool SatisfySignature(const std::string& name, std::set<SnippetKind> types);

  /* meta data */
  std::string GetCode() const {return m_code;}
  int GetLineNumber() const {return m_line_number;}
  std::string GetFileName() const {return m_filename;}
  int GetLOC() const {return m_loc;}
  bool IsValid() const {return !m_code.empty();}

  std::string ToString() const;
private:
  snippet_signature m_sig;
  std::string m_main_name;
  SnippetKind m_main_kind;
  std::string m_code;
  int m_line_number; // TODO NOW
  std::string m_filename;
  int m_loc;
};

std::vector<std::string> query_code(const std::string& code, const std::string& query);
std::string query_code_first(const std::string& code, const std::string& query);


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
  static void check_headers();
private:
  SystemResolver();
  ~SystemResolver() {}
  // std::vector<Header> m_headers; // header files used
  static SystemResolver* m_instance;
  tagFile *m_tagfile = NULL;
  tagEntry *m_entry = NULL;
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

  /* resolving */
  std::set<Snippet*> Resolve(const std::string& name);
  std::set<Snippet*> Resolve(const std::string& name, SnippetKind kind);
  std::set<Snippet*> Resolve(const std::string& name, std::set<SnippetKind> kinds);

  std::set<Snippet*> GetDeps(Snippet* snippet);
  // recursively get dependence
  std::set<Snippet*> GetAllDeps(Snippet* snippet);
  std::set<Snippet*> GetAllDeps(std::set<Snippet*> snippets);

  std::string ToString() const;

private:
  /* lookup */
  std::set<Snippet*> lookUp(const std::string& name);
  std::set<Snippet*> lookUp(const std::string& name, SnippetKind kind);
  std::set<Snippet*> lookUp(const std::string& name, std::set<SnippetKind> kinds);
  
  // Can not add dependence outside the class
  void addDep(Snippet *from, Snippet *to);
  void addDeps(Snippet *from, std::set<Snippet*> to);
  
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


/*******************************
 ** Subclass snipet
 *******************************/

class FunctionSnippet : public Snippet {
public:
  std::string GetDecl() const;
};

#endif
