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


class Snippet {
public:
  Snippet() {}
  ~Snippet() {}
  std::string GetName() {return m_name;}
  char GetType() {return m_type;}
  std::string GetCode() {return m_code;}
private:
  std::string m_code;
  std::string m_name;
  char m_type;
};

#endif
