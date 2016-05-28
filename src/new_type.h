#ifndef NEW_TYPE_H
#define NEW_TYPE_H

#include <string>
#include "ast.h"
#include "snippet.h"

class TestInput;
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
  virtual std::string GetInputCode(std::string var) = 0;
  virtual std::string GetOutputCode(std::string var) = 0;
  // GetTestInput is deprecated. Use GetTestInputSpec instead
  virtual std::vector<std::string> GetTestInput(int size) {
    std::vector<std::string> ret;
    for (int i=0;i<size;i++) {
      ret.push_back(GetTestInput());
    }
    return ret;
  }
  virtual std::string GetTestInput() = 0;
  /**
   * CAUTION The TestInput* needs to be free-d by the user.
   * FIXME NOW this should be called GenerateXXX, so that it reminds user something is generated, and needs to be free-d
   */
  virtual std::vector<TestInput*> GetTestInputSpec(std::string var, int size) {
    std::vector<TestInput*> ret;
    for (int i=0;i<size;i++) {
      ret.push_back(GetTestInputSpec(var));
    }
    return ret;
  }
  virtual TestInput* GetTestInputSpec(std::string var);
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
  virtual std::string GetOutputCode(std::string var) override;
  virtual std::string GetTestInput() override;
  // virtual TestInput GetTestInputSpec() override;
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
  virtual std::string GetOutputCode(std::string var) override;
  virtual std::string GetTestInput() override;
  // virtual TestInput GetTestInputSpec() override;
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
  virtual std::string GetOutputCode(std::string var) override = 0;
  virtual std::string GetTestInput() override = 0;
  // virtual TestInput GetTestInputSpec() override;
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
  virtual std::string GetOutputCode(std::string var) override;
  virtual std::string GetTestInput() override;
  virtual TestInput* GetTestInputSpec(std::string var) override;
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
  virtual std::string GetOutputCode(std::string var) override;
  virtual std::string GetTestInput() override;
  // virtual TestInput* GetTestInputSpec(std::string var) override;
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
  virtual std::string GetOutputCode(std::string var) override;
  virtual std::string GetTestInput() override;
  // virtual TestInput GetTestInputSpec() override;
};
class Void : public PrimitiveNewType {
public:
  Void(std::string raw, std::vector<std::string> dims = {}) : PrimitiveNewType(raw, dims) {}
  virtual ~Void() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual std::string GetTestInput() override;
  // virtual TestInput GetTestInputSpec() override;
};
class Bool : public PrimitiveNewType {
public:
  Bool(std::string raw, std::vector<std::string> dims = {}) : PrimitiveNewType(raw, dims) {}
  virtual ~Bool() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual std::string GetTestInput() override;
  // virtual TestInput GetTestInputSpec() override;
};

class NewVariable {
public:
  NewVariable(NewType *type, std::string var) : m_type(type), m_var(var) {}
  NewType *GetType() {return m_type;}
  std::string GetName() {return m_var;}
private:
  NewType *m_type;
  std::string m_var;
};

/**
 * Test input class.
 * Test input should not just be a string.
 * it should be able to dump, be able to describe the meta data of this test case,
 * e.g. size of the buffer array, value of a int, generated string length.
 */
class TestInput {
public:
  TestInput(NewType *type, std::string var) : m_type(type), m_var(var) {}
  virtual ~TestInput() {}
  std::string GetRaw() {return m_raw;}
  std::string GetVar() {return m_var;}
  NewType *GetType() {return m_type;}
  void SetRaw(std::string raw) {m_raw = raw;}
  virtual std::string dump() {
    std::string ret;
    ret += "Default TestInput\n";
    ret += m_type->ToString() + " " + m_var + "\n";
    return ret;
  }
  /**
   * Must be of xxx=yyy format, in each line
   */
  virtual std::string ToString() {
    std::string ret;
    ret += m_var + " = Default\n";
    return ret;
  }
protected:
  NewType *m_type;
  std::string m_var;
  std::string m_raw;
};

class CharTestInput : public TestInput {
public:
  CharTestInput(NewType *type, std::string var) : TestInput(type, var) {}
  void SetStrlen(std::vector<int> strlens) {
    m_strlens = strlens;
  }
  virtual std::string dump() {
    std::string ret;
    ret += m_type->ToString() + " " + m_var + "\n";
    ret += "size: " + std::to_string(m_strlens.size()) + ", ";
    ret += "strlens:";
    for (int len : m_strlens) {
      ret += " " + std::to_string(len);
    }
    return ret;
  }
  virtual std::string ToString() {
    std::string ret;
    ret += m_var + ".size() = " + std::to_string(m_strlens.size()) + "\n";
    for (int i=0;i<(int)m_strlens.size();++i) {
      ret += "strlen[" + std::to_string(i) + "] = " + std::to_string(m_strlens[i]) + "\n";
    }
    return ret;
  }
private:
  /**
   * If only one pointer, the strlen only has one size, and contains one integer.
   * If pointer to pointer, it will have size 1, and the inner have size many
   * If pointer to pointer to pointer, it will have size 3, etc.
   * TODO right now, we only handle char**, no more
   * Thus the strlens only need one dimension
   */
  std::vector<int> m_strlens;
};


#endif /* NEW_TYPE_H */
