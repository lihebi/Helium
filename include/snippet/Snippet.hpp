#ifndef __SNIPPET_HPP__
#define __SNIPPET_HPP__

#include <string>
#include <vector>

enum snippet_type {
  FUNCTION,
  STRUCTURE,
  ENUM,
  VARIABLE,
  DEFINE
};
class Snippet {
public:
  Snippet();
  ~Snippet();
  virtual void GetName();
  virtual void GetCode();
  virtual void GetDependence();
private:
  std::string m_code;
  std::string m_name;
  enum snippet_type m_type;
  std::vector<std::string> dependence;
};

#endif
