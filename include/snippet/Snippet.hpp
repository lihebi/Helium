#ifndef __SNIPPET_HPP__
#define __SNIPPET_HPP__

#include <string>
#include <vector>
#include <set>

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
  virtual ~Snippet() {}
  virtual std::string GetName() = 0;
  virtual char GetType() = 0;
  virtual std::string GetCode() = 0;
  virtual std::set<std::string> GetKeywords() = 0;
  virtual std::string GetFilename() const = 0;
  virtual int GetLineNumber() const = 0;
private:
};

#endif
