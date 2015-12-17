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
  virtual std::string GetDecl() {return "";}
  virtual int GetLOC() const = 0;

  // functions for get code
  static std::string GetFunctionCode(std::string filename, int line, std::string function_name="");
  static std::string GetEnumCode(std::string filename, int line, std::string enum_name="");
  static std::string GetDefineCode(std::string filename, int line);

  static std::string GetStructCode(std::string filename, int line, std::string name, std::string alias);
  static std::string GetUnionCode(std::string filename, int line, std::string name, std::string alias);
  static std::string GetVariableCode(std::string filename, int line, std::string name);


  static std::string GetCode(std::string filename, int line, std::string name, std::string alias, std::string tag);
  static std::string GetTypedefCode(std::string filename, int line, std::string alias);
  
private:
};

#endif
