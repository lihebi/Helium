#ifndef __CTAGS_HPP__
#define __CTAGS_HPP__

#include <string>
#include <readtags.h>
#include <vector>
#include "snippet/Snippet.hpp"

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
  std::string GetTyperef() const {return m_typeref;}
private:
  std::string m_name;
  std::string m_file;
  std::string m_simple_filename; // the last portion of path. Use for header sorter
  // the pattern is not used for find code.
  // The pattern itself is used in system resolver to recursively resolve type to primitive
  std::string m_pattern;
  int m_line;
  char m_type;
  std::string m_typeref;
};

class Ctags {
public:
  static Ctags* Instance() {
    if (m_instance == 0) {
      m_instance = new Ctags();
    }
    return m_instance;
  }
  void Load(const std::string& tagfile);
  // return the first occurrence of name
  CtagsEntry ParseSimple(const std::string& name);
  // return all occurrence of name
  std::vector<CtagsEntry> Parse(const std::string& name);
  // return all the occurence of name of t in type
  std::vector<CtagsEntry> Parse(const std::string& name, const std::string& type);
  Snippet* ResolveSimple(const std::string& name);
  // not just get the CtagsEntry, but get the code block of <name>
  std::set<Snippet*> Resolve(const std::string& name);
  // resolve by type
  std::set<Snippet*> Resolve(const std::string& name, const std::string& type);
private:
  Ctags() {}
  ~Ctags() {}
  static Ctags* m_instance;
  tagFile *m_tagfile;
  tagEntry *m_entry;
};

#endif
