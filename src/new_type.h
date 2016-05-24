#ifndef NEW_TYPE_H
#define NEW_TYPE_H

#include <string>
#include "ast.h"
#include "snippet.h"

/**
 * This is the new type system.
 * Changing the old one makes the whole system down.
 */

typedef enum _NewTypeKind {
  NTK_Unknown,
  // TK_Enum,
  NTK_Primitive,
  // TK_Structure,
  NTK_System,
  // TK_Union
  NTK_Local,
} NewTypeKind;


class NewType;

/**
 * How to create NewType?
 * 1. give the type name, the dimension information
 
      Why I need to separate the dimension?
      Because the dimension information often attached after the variable name.
      Well, I would say always.
      This will cause problem for a source level parser, like SrcML.
      More specifically, srcml gives <type> tag, but the dimension information is outside.
      
      But you can leave it empty.
      Also, the dimension itself might not be a constant number.
      It is very possible for it to be a macro.
      But it can also be a variable expression? I don't think so.
      So, just include whatever it appears.
      FIXME Do not forget to resolve the snippet for the macro!
      
      TODO For the allocation code, do we need to include them?
      
   2. directly from an XML node, of SrcML.
      This node should be a <decl>.
      TODO to confirm: in srcml, is there any non-<decl> node contains a type?

 * Note: all the NewType* created should be free-d after use, by the user.
 */
class NewTypeFactory {
public:
  /**
   * @param [in] dims for a[5][MAX], it would be {"5", "MAX"}
   */
  static NewType* CreateType(std::string raw, std::vector<std::string> dims);
  /**
   * This should not be a <type> node, because it does not include the dimention information.
   */
  static NewType* CreateType(ast::XMLNode decl_node);
private:
};

class NewType {
public:
  NewType(std::string raw, std::vector<std::string> dims = {});
  virtual ~NewType() {}
  virtual std::string ToString() const = 0;
  virtual NewTypeKind Kind() const = 0;
  std::string Raw() const {return m_raw;}
  /**
   * Dimension string used after var name.
   * e.g. int aaa[3];
   */
  std::string DimensionSuffix() const {
    std::string ret;
    for (std::string dim : m_dims) {
      ret += "[" + dim + "]";
    }
    return ret;
  }
  int Pointer() const {return m_pointer;}
  int Dimension() const {return m_dims.size();}
  friend std::string get_input_code(NewType type, const std::string &name);
  friend std::string get_random_input(NewType type);

  /**
   * Before the input, we need decl the variable.
   */
  virtual std::string GetDeclCode(std::string var) = 0;
  /**
   * Get the input
   */
  virtual std::string GetInputCode(std::string varname) = 0;
protected:
  std::string m_raw;
  std::vector<std::string> m_dims;
  int m_pointer;
  int m_dimension;
  std::string getInputDecl(std::string varname);
};

class SystemNewType : public NewType {
public:
  SystemNewType(std::string raw, std::vector<std::string> dims = {}) : NewType(raw, dims) {}
  ~SystemNewType() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual NewTypeKind Kind() const override {
    return NTK_System;
  }
  virtual std::string ToString() const override {
    std::string ret;
    ret += "SystemNewType: " + m_raw;
    for (std::string dim : m_dims) {
      ret += "[" + dim + "]";
    }
    return ret;
  }
private:
};

class LocalNewType : public NewType {
public:
  LocalNewType(std::string raw, std::vector<std::string> dims = {}) : NewType(raw, dims) {}
  ~LocalNewType() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual NewTypeKind Kind() const override {
    return NTK_Local;
  }
  virtual std::string ToString() const override {
    std::string ret;
    ret += "LocalNewType: " + m_raw;
    for (std::string dim : m_dims) {
      ret += "[" + dim + "]";
    }
    return ret;
  }
private:
};

/********************************
 * Primitive NewType
 *******************************/

/**
 * This class should not be initialized.
 * Use its children classes instead.
 * Consider use abstract class to enforce it.
 */
class PrimitiveNewType : public NewType {
public:
  PrimitiveNewType(std::string raw, std::vector<std::string> dims = {}) : NewType(raw, dims) {}
  ~PrimitiveNewType() {}
  virtual std::string GetInputCode(std::string var) override = 0;
  virtual std::string GetDeclCode(std::string var) override = 0;
  virtual NewTypeKind Kind() const override {
    return NTK_Primitive;
  }
  virtual std::string ToString() const override {
    std::string ret;
    ret += "PrimitiveNewType: " + m_raw;
    for (std::string dim : m_dims) {
      ret += "[" + dim + "]";
    }
    return ret;
  }
};

class Char : public PrimitiveNewType {
public:
  Char(std::string raw, std::vector<std::string> dims = {}) : PrimitiveNewType(raw, dims) {}
  virtual ~Char() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
private:
};

/**
 * short, long, long long
 * signed, unsigned
 */
class Int : public PrimitiveNewType {
public:
  Int(std::string raw, std::vector<std::string> dims = {}) : PrimitiveNewType(raw, dims) {}
  virtual ~Int() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
};
/**
 * float, double
 */
class Float : public PrimitiveNewType {
public:
  Float(std::string raw, std::vector<std::string> dims = {}) : PrimitiveNewType(raw, dims) {}
  virtual ~Float() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
};
class Void : public PrimitiveNewType {
public:
  Void(std::string raw, std::vector<std::string> dims = {}) : PrimitiveNewType(raw, dims) {}
  virtual ~Void() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
};
class Bool : public PrimitiveNewType {
public:
  Bool(std::string raw, std::vector<std::string> dims = {}) : PrimitiveNewType(raw, dims) {}
  virtual ~Bool() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
};


#endif /* NEW_TYPE_H */
