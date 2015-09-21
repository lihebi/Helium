#ifndef __ENUM_SNIPPET_HPP__
#define __ENUM_SNIPPET_HPP__

#include "snippet/Snippet.hpp"

class EnumSnippet : public Snippet {
public:
  EnumSnippet(const std::string& code);
  virtual ~EnumSnippet() {}
  virtual std::string GetName() {return m_name;}
  virtual char GetType() {return m_type;}
  virtual std::string GetCode() {return m_code;}
  virtual std::set<std::string> GetKeywords() {return m_keywords;}
private:
  std::string m_code;
  std::string m_name;
  std::string m_alias;
  char m_type;
  std::set<std::string> m_keywords;
};

#endif
