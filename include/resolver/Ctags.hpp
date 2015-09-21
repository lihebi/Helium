#ifndef __CTAGS_HPP__
#define __CTAGS_HPP__

#include <string>
#include <readtags.h>
#include <vector>
#include "snippet/Snippet.hpp"

// we need: 1. file path 2. line number 3. type
class CtagsEntry {
public:
  CtagsEntry(const std::string& file, int line, char type)
  : m_file(file), m_line(line), m_type(type), m_valid(true) {}
  CtagsEntry(const char* file, int line, char type)
  : m_file(file), m_line(line), m_type(type), m_valid(true) {}
  CtagsEntry(bool valid) : m_valid(valid) {}
  ~CtagsEntry() {}
  std::string GetFileName() const {
    return m_file;
  }
  int GetLineNumber() const {
    return m_line;
  }
  char GetType() const {
    return m_type;
  }
  operator bool() const {
    return m_valid;
  }
private:
  std::string m_file;
  int m_line;
  char m_type;
  bool m_valid;
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
