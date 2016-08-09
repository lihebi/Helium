#ifndef TYPE_H
#define TYPE_H

#include <string>
#include "parser/xmlnode.h"
#include "resolver/snippet.h"

class TestInput;
/**
 * This is the new type system.
 * Changing the old one makes the whole system down.
 */

typedef enum _TypeKind {
  NTK_Unknown,
  // TK_Enum,
  NTK_Primitive,
  // TK_Structure,
  NTK_System,
  // TK_Union
  NTK_Local,
  NTK_LocalStructure,
  NTK_LocalTypedef,
  NTK_LocalEnum
} TypeKind;


class Type;

/**
 * How to create Type?
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

 * Note: all the Type* created should be free-d after use, by the user.
 */
class TypeFactory {
public:
  /**
   * @param [in] dims for a[5][MAX], it would be {"5", "MAX"}
   * @param token how many times remaining to inspect type's fields
   * Default token is 2!
   */
  static Type* CreateType(std::string raw, std::vector<std::string> dims, int token = 2);
  /**
   * This should not be a <type> node, because it does not include the dimention information.
   */
  static Type* CreateType(XMLNode decl_node, int token = 2);
private:
};

class Type {
public:
  Type(std::string raw, std::vector<std::string> dims = {});
  virtual ~Type() {}
  virtual std::string ToString() const = 0;
  virtual TypeKind Kind() const = 0;
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
  // friend std::string get_input_code(Type type, const std::string &name);
  // friend std::string get_random_input(Type type);

  /**
   * Before the input, we need decl the variable.
   */
  virtual std::string GetDeclCode(std::string var) = 0;
  /**
   * Get the input
   */
  virtual std::string GetInputCode(std::string var) = 0;
  virtual std::string GetOutputCode(std::string var) = 0;
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
  std::string m_raw; // the raw type string, right before the variable name
  std::string m_id; // the id of raw, removing the common keywords like int, const
  std::string m_raw_without_pointer; // remove the tail '*'s
  std::vector<std::string> m_dims;
  int m_pointer;
  int m_dimension;
  std::string getInputDecl(std::string varname);
};

class SystemType : public Type {
public:
  SystemType(std::string raw, std::vector<std::string> dims = {}) : Type(raw, dims) {}
  ~SystemType() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  // virtual TestInput GetTestInputSpec() override;
  virtual TypeKind Kind() const override {
    return NTK_System;
  }
  virtual std::string ToString() const override {
    std::string ret;
    ret += "SystemType: " + m_raw;
    for (std::string dim : m_dims) {
      ret += "[" + dim + "]";
    }
    return ret;
  }
private:
};

/**
 * DEPRECATED use the specific type kind class
 */
class LocalType : public Type {
public:
  LocalType(std::string raw, std::vector<std::string> dims = {});
  ~LocalType() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual TestInput* GetTestInputSpec(std::string var) override;
  virtual TypeKind Kind() const override {
    return NTK_Local;
  }
  virtual std::string ToString() const override {
    std::string ret;
    ret += "LocalType: " + m_raw;
    for (std::string dim : m_dims) {
      ret += "[" + dim + "]";
    }
    return ret;
  }
private:
  int m_snippet_id = -1;
};

class LocalTypedefType : public Type {
public:
  LocalTypedefType(int snippet_id, std::string raw, std::vector<std::string> dims = {}, int token = 0);
  ~LocalTypedefType();
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual TestInput* GetTestInputSpec(std::string var) override;
  virtual TypeKind Kind() const override {
    return NTK_LocalTypedef;
  }
  virtual std::string ToString() const override {
    std::string ret;
    ret += "LocalTypedefType: " + m_raw;
    for (std::string dim : m_dims) {
      ret += "[" + dim = "]";
    }
    return ret;
  }
private:
  Type *m_next;
  int m_snippet_id = -1;
  int m_token = 0;
  std::string m_code;
};

class LocalStructureType : public Type {
public:
  LocalStructureType(int snippet_id, std::string raw, std::vector<std::string> dims = {}, int token = 0);
  ~LocalStructureType() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual TestInput* GetTestInputSpec(std::string var) override;
  virtual TypeKind Kind() const override {
    return NTK_LocalStructure;
  }
  virtual std::string ToString() const override {
    std::string ret;
    ret += "LocalStructureType: " + m_raw;
    for (std::string dim : m_dims) {
      ret += "[" + dim + "]";
    }
    return ret;
  }

  std::vector<std::pair<Type*, std::string> > Fields() {return m_fields;}
private:
  void buildArch();
  // if this token is 0, stop recuring its fields. Stop actually analyze its fields.
  // That is, when the token is 0, the type should be a pointer, and set to NULL
  int m_snippet_id = -1;
  int m_token = 0;
  std::string m_code;
  std::vector<std::pair<Type*, std::string> > m_fields; // type and variable name
};

class LocalEnumType : public Type {
public:
  LocalEnumType(int snippet_id, std::string raw, std::vector<std::string> dims = {});
  ~LocalEnumType() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual TestInput* GetTestInputSpec(std::string var) override;
  virtual TypeKind Kind() const override {
    return NTK_LocalEnum;
  }
  virtual std::string ToString() const override {
    std::string ret;
    ret += "LocalStructureType: " + m_raw;
    for (std::string dim : m_dims) {
      ret += "[" + dim + "]";
    }
    return ret;
  }
private:
  int m_snippet_id = -1;
};


/********************************
 * Primitive Type
 *******************************/

/**
 * This class should not be initialized.
 * Use its children classes instead.
 * Consider use abstract class to enforce it.
 */
class PrimitiveType : public Type {
public:
  PrimitiveType(std::string raw, std::vector<std::string> dims = {}) : Type(raw, dims) {}
  ~PrimitiveType() {}
  virtual std::string GetInputCode(std::string var) override = 0;
  virtual std::string GetDeclCode(std::string var) override = 0;
  virtual std::string GetOutputCode(std::string var) override = 0;
  // virtual TestInput GetTestInputSpec() override;
  virtual TypeKind Kind() const override {
    return NTK_Primitive;
  }
  virtual std::string ToString() const override {
    std::string ret;
    ret += "PrimitiveType: " + m_raw;
    for (std::string dim : m_dims) {
      ret += "[" + dim + "]";
    }
    return ret;
  }
};

class Char : public PrimitiveType {
public:
  Char(std::string raw, std::vector<std::string> dims = {}) : PrimitiveType(raw, dims) {}
  virtual ~Char() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual TestInput* GetTestInputSpec(std::string var) override;
private:
  std::string getOutputCode_Zero(std::string var); // zero pointer
  std::string getOutputCode_One(std::string var); // 1 dimensional pointer
  std::string getOutputCode_Two(std::string var); // 2 dimensional pointer
};

/**
 * short, long, long long
 * signed, unsigned
 */
class Int : public PrimitiveType {
public:
  Int(std::string raw, std::vector<std::string> dims = {}) : PrimitiveType(raw, dims) {}
  virtual ~Int() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual TestInput* GetTestInputSpec(std::string var) override;
};
/**
 * float, double
 */
class Float : public PrimitiveType {
public:
  Float(std::string raw, std::vector<std::string> dims = {}) : PrimitiveType(raw, dims) {}
  virtual ~Float() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
};
class Void : public PrimitiveType {
public:
  Void(std::string raw, std::vector<std::string> dims = {}) : PrimitiveType(raw, dims) {}
  virtual ~Void() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
};
class Bool : public PrimitiveType {
public:
  Bool(std::string raw, std::vector<std::string> dims = {}) : PrimitiveType(raw, dims) {}
  virtual ~Bool() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual TestInput* GetTestInputSpec(std::string var) override;
};


class UnknownType : public Type {
public:
  UnknownType(std::string raw, std::vector<std::string> dims = {}) : Type(raw, dims) {}
  ~UnknownType() {}
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual TypeKind Kind() const override {
    return NTK_Unknown;
  }
  virtual std::string ToString() const override {
    return "Unknown type: " + m_raw;
  }
private:
};

/**
 * Test input class.
 * Test input should not just be a string.
 * it should be able to dump, be able to describe the meta data of this test case,
 * e.g. size of the buffer array, value of a int, generated string length.
 */
class TestInput {
public:
  TestInput(Type *type, std::string var) : m_type(type), m_var(var) {}
  virtual ~TestInput() {}
  std::string GetRaw() {return m_raw;}
  std::string GetVar() {return m_var;}
  Type *GetType() {return m_type;}
  void SetRaw(std::string raw) {m_raw = raw;}
  /**
   * For human read only.
   */
  virtual std::string dump();
  /**
   * Must be of xxx=yyy format, in each line
   */
  virtual std::string ToString();
protected:
  Type *m_type;
  std::string m_var;
  // the raw data is used as the input to the executable under test
  std::string m_raw;
};

typedef enum _NullKind {
  NULL_NA,
  NULL_True,
  NULL_False
} NullKind;

class BoolTestInput : public TestInput {
public:
  BoolTestInput(Type *type, std::string var) : TestInput(type, var) {}
  virtual std::string dump();
  virtual std::string ToString();
  void SetValue(bool v) {m_v = v;}
private:
  bool m_v;
};

class LocalTestInput : public TestInput {
public:
  LocalTestInput(Type *type, std::string var) : TestInput(type, var) {}
  virtual std::string dump();
  virtual std::string ToString();
  // must be set
  void SetNull(NullKind null) {
    m_null = null;
  }
private:
  // if this string is empty, the result is NA
  NullKind m_null;
};

class LocalStructTestInput : public TestInput {
public:
  LocalStructTestInput(Type *type, std::string var) : TestInput(type, var) {}
  virtual std::string dump();
  virtual std::string ToString();
  void SetNull(NullKind null) {
    m_null = null;
  }
  void AddField(TestInput *in) {
    m_field_inputs.push_back(in);
  }
private:
  NullKind m_null; // FIXME must make sure this is set
  std::vector<TestInput*> m_field_inputs;
};

class IntTestInput : public TestInput {
public:
  IntTestInput(Type *type, std::string var) : TestInput(type, var) {}
  virtual std::string dump();
  virtual std::string ToString();
  // TODO for pointer, just ignore the value
  // FIXME should make sure these setters are called for each IntTestInput instance
  void SetPointer(int p) {m_pointer = p;}
  void SetValue(int v) {m_value = v;}
private:
  int m_pointer = 0;
  int m_value = -1;
};

class CharTestInput : public TestInput {
public:
  CharTestInput(Type *type, std::string var) : TestInput(type, var) {}
  void SetStrlen(std::vector<int> strlens) {m_strlens = strlens;}
  void SetBufSize(std::vector<int> bufsizs) {m_bufsizs = bufsizs;}
  virtual std::string dump();
  virtual std::string ToString();
private:
  /**
   * If only one pointer, the strlen only has one size, and contains one integer.
   * If pointer to pointer, it will have size 1, and the inner have size many
   * If pointer to pointer to pointer, it will have size 3, etc.
   * TODO right now, we only handle char**, no more
   * Thus the strlens only need one dimension
   */
  std::vector<int> m_strlens;
  std::vector<int> m_bufsizs;
};

class ArgVTestInput : public TestInput {
public:
  ArgVTestInput(Type *type, std::string var) : TestInput(type, var) {}
  void SetSpec(std::string spec) {m_spec = spec;}
  virtual std::string dump() {
    return "this is argV test input";
  }
  virtual std::string ToString() {
    return m_spec;
  }
private:
  std::string m_spec;
};

class ArgCTestInput : public TestInput {
public:
  ArgCTestInput() : TestInput(NULL, "argc") {}
  void SetSpec(std::string spec) {m_spec = spec;}
  virtual std::string dump() {
    return "ARG C input";
  }
  virtual std::string ToString() {
    return m_spec;
  }
private:
  std::string m_spec;
};

class ArgCV {
public:
  // "achtvf:"
  // only support single colon
  void SetOpt(std::string opt) {
    m_opt = opt;
    for (int i=opt.length()-1;i>=0;i--) {
      if (opt[i] == ':') {
        i--;
        m_str_opt.insert(opt[i]);
      } else {
        m_bool_opt.insert(opt[i]);
      }
    }
  }
  std::pair<TestInput*, TestInput*> GetTestInputSpec();
  std::vector<std::pair<TestInput*, TestInput*> > GetTestInputSpec(int number);
private:
  std::string m_opt;
  std::set<char> m_bool_opt; // ach
  std::set<char> m_str_opt; // f
};


#endif /* TYPE_H */
