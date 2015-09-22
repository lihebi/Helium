#ifndef __STRUCTURE_SNIPPET_HPP__
#define __STRUCTURE_SNIPPET_HPP__

#include "snippet/Snippet.hpp"

class StructureSnippet : public Snippet {
public:
  StructureSnippet(const std::string& code, const std::string& filename, int line_number);
  virtual ~StructureSnippet() {}
  // this name should be legal to define a variable
  virtual std::string GetName() {
    if (m_name.empty()) {
      return m_alias;
    } else {
      return "struct " + m_name;
    }
  }
  virtual char GetType() {return m_type;}
  virtual std::string GetCode() {return m_code;}
  virtual std::set<std::string> GetKeywords() {return m_keywords;}
  virtual std::string GetFilename() const {return m_filename;}
  virtual int GetLineNumber() const {return m_line_number;}
private:
  std::string m_code;
  // name is only something that can print out as debug information
  std::string m_name;
  std::string m_alias;
  char m_type;
  std::string m_filename;
  int m_line_number;
  std::set<std::string> m_keywords;
};

#endif
