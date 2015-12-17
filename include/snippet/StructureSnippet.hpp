#ifndef __STRUCTURE_SNIPPET_HPP__
#define __STRUCTURE_SNIPPET_HPP__

#include "snippet/Snippet.hpp"
#include "resolver/Ctags.hpp"

class StructureSnippet : public Snippet {
public:
  StructureSnippet(const std::string& code, const std::string& filename, int line_number);
  StructureSnippet(const CtagsEntry& ce);
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
  virtual int GetLOC() const {return m_loc;}
private:
  void getName(const CtagsEntry& ce);
  std::string getStructureCode(std::string filename, int line);
  void print();
  
  std::string m_code;
  // m_name for structuresnippet should be the true name
  std::string m_name;
  std::string m_alias;
  char m_type;
  std::string m_filename;
  int m_line_number = 0;
  int m_loc = 0;
  std::set<std::string> m_keywords;
};

#endif
