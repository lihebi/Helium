#ifndef __VARIABLE_SNIPPET_HPP__
#define __VARIABLE_SNIPPET_HPP__

#include "snippet/Snippet.hpp"
#include "resolver/Ctags.hpp"

class VariableSnippet : public Snippet {
public:
  VariableSnippet(const std::string& code, const std::string& id, const std::string& filename, int line_number);
  VariableSnippet(const CtagsEntry& ce);
  virtual ~VariableSnippet() {}
  virtual std::string GetName() {return m_name;}
  virtual char GetType() {return m_type;}
  virtual std::string GetCode() {return m_code;}
  virtual std::set<std::string> GetKeywords() {return m_keywords;}
  virtual std::string GetFilename() const {return m_filename;}
  virtual int GetLineNumber() const {return m_line_number;}
  virtual int GetLOC() const {return m_loc;}
private:
  std::string m_code;
  std::string m_name;
  char m_type;
  std::string m_filename;
  int m_line_number = 0;
  std::set<std::string> m_keywords;
  int m_loc = 0;
};

#endif
