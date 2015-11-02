#ifndef __TYPEDEF_SNIPPET_HPP__
#define __TYPEDEF_SNIPPET_HPP__

#include "snippet/Snippet.hpp"
#include "resolver/Ctags.hpp"

enum typedef_type {
  TYPEDEF_TYPE,
  TYPEDEF_FUNC_POINTER
};

class TypedefSnippet : public Snippet {
public:
  TypedefSnippet(const std::string& code, const std::string& id, const std::string& filename, int line_number);
  TypedefSnippet(const CtagsEntry& ce);
  virtual ~TypedefSnippet() {}
  virtual std::string GetName() {return m_name;}
  virtual char GetType() {return m_type;}
  virtual std::string GetCode() {return m_code;}
  virtual std::set<std::string> GetKeywords() {return m_keywords;}
  virtual std::string GetFilename() const {return m_filename;}
  virtual int GetLineNumber() const {return m_line_number;}
  virtual int GetLOC() const {return m_loc;}

  // own functions
  enum typedef_type GetTypedefType() const {return m_typedef_type;}
  std::string GetToType() const {return m_to;}
private:
  void semanticParse();
  std::string m_code;
  std::string m_name;
  std::string m_from; // typedef name: should be the same as m_name
  std::string m_to; // what to substitute the name
  // FIXME what is the init value of this data member? It may be 0, the first type of the enum!!
  enum typedef_type m_typedef_type; // merely type convert or, a function pointer, or something else ..
  char m_type;
  std::string m_filename;
  int m_line_number = 0;
  std::set<std::string> m_keywords;
  int m_loc = 0;
};

#endif
