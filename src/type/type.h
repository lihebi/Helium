#ifndef TYPE_H
#define TYPE_H

#include "common.h"
#include "parser/xmlnode.h"
#include "utils/string_utils.h"

#include "input_spec.h"

class Type;
/**
 * StrType
 * BufType
 * ArgCV
 *
 * PointerType
 * ArrayType
 *
 * PrimitiveType
 * - Int
 * - Char
 *
 * System
 * Structure
 */
class TypeFactory {
public:
  static Type *CreateType(XMLNode decl_node);
  static Type *CreateType(std::string str);
};


// class ArrayTypeFactory {
// public:
//   static Type *CreateType(std::string str);
// };

// class PointerTypeFactory {
// public:
//   static Type *CreateType(std::string str);
// };

class Type {
public:
  Type() {}
  virtual ~Type() {}
  virtual InputSpec* GenerateRandomInput(bool simple=false) = 0;
  std::vector<InputSpec*> GenerateRandomInputs(int num=1) {
    std::vector<InputSpec*> ret;
    while (num-- > 0) {
      ret.push_back(GenerateRandomInput());
    }
    return ret;
  }
  virtual std::string GetDeclCode(std::string var) = 0;
  /**
   * @param [in] simple whether to generate simple input.
   * This is used to solve the recursive structure problem.
   * This is currently only apply to sequential type
   * If it is the pointer type, it will not recursively generate for the type.
   * Instead, only NULL or not NULL(malloc)sizeof(A)*1 is used.
   */
  virtual std::string GetInputCode(std::string var, bool simple=false) = 0;
  virtual std::string GetOutputCode(std::string var, bool simple=false) = 0;
  virtual void GenerateIOFunc() = 0;
  virtual std::vector<InputSpec*> GenerateCornerInputs(int limit=-1);
  static std::string GetHeader();
  // overwrite when possible!
  // The default implementaiton is just generate multiple times
  virtual std::vector<InputSpec*> GeneratePairInput() {
    std::vector<InputSpec*> ret;
    // TODO magic number
    for (int i=0;i<10;i++) {
      InputSpec *spec = GenerateRandomInput();
      ret.push_back(spec);
    }
    return ret;
  }

  // Get the "raw" of the type, e.g. "char"
  // this is proposed for "char* xx = malloc(sizeof(char) * num)". Note that sizeof(char)
  // TODO what's this?
  // virtual std::string GetValue() = 0;
  virtual std::string GetRaw() = 0;
  virtual std::string ToString() = 0;

protected:
  std::vector<InputSpec*> m_corners;
private:
};

/**
 * First level
 */
class SystemType : public Type {
public:
  SystemType() {}
  virtual ~SystemType() {}
protected:
private:
};

class PrimitiveType : public Type {
public:
  PrimitiveType() {}
  virtual ~PrimitiveType() {}
protected:
private:
};

class CompositeType : public Type {
public:
  CompositeType() {}
  virtual ~CompositeType() {}
protected:
private:
};

// class SpecialType : public Type {
// public:
//   SpecialType() {}
//   virtual ~SpecialType() {}
// protected:
// private:
// };

class UnknownType : public Type {
public:
  UnknownType(std::string str) : m_raw(str) {}
  virtual ~UnknownType() {}
  virtual InputSpec* GenerateRandomInput(bool simple=false) { return NULL;}
  virtual std::string GetDeclCode(std::string var) {
    std::string ret;
    ret += "// UnknownType::GetDeclCode: " + var + ";\n";
    ret += m_raw + " " + var + ";\n";
    return ret;
  }
  virtual std::string GetInputCode(std::string var, bool simple=false) {
    var.empty();
    return "";
  }
  virtual std::string GetOutputCode(std::string var, bool simple=false) {
    var.empty();
    return "";
  }
  virtual std::string GetRaw() {return m_raw;}
  virtual std::string ToString() {
    return "UnknownType: " + m_raw;
  }
private:
  std::string m_raw;
};

/**
 * Second level
 */

class SequentialType : public CompositeType {
public:
  SequentialType() {}
  virtual ~SequentialType() {}
protected:
private:
};

class StructType : public CompositeType {
public:
  StructType(std::string raw, int id);
  virtual ~StructType() {}
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string ToString() override {return "SturctType";}
  virtual std::string GetInputCode(std::string var, bool simple=false) override;
  virtual std::string GetOutputCode(std::string var, bool simple=false) override;
  virtual void GenerateIOFunc() override;
  virtual InputSpec *GenerateRandomInput(bool simple=false) override;
  virtual std::string GetRaw() override;
protected:
private:
  std::string m_raw;
  int m_snippet_id = -1;
};

/**
 * Third level
 */

/**
 * ArrayType
 * When generating input, need to look into contained type to see if that is a "char".
 */
class ArrayType : public SequentialType {
public:
  ArrayType(std::string type_str, int num);
  ~ArrayType();
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var, bool simple=false) override;
  virtual std::string GetOutputCode(std::string var, bool simple=false) override;
  virtual InputSpec *GenerateRandomInput(bool simple=false) override;
  virtual std::string GetRaw() override {
    std::string ret;
    if (m_contained_type) {
      ret += m_contained_type->GetRaw() + "[" + std::to_string(m_num) + "]";
    }
    return ret;
  }
  virtual std::string ToString() override {return "ArrayType";}
protected:
  Type *m_contained_type = NULL;
  int m_num = 0; // number
private:
};

/**
 * Pointer type.
 * When generating input, need to look into contained type to see if that is a "char".
 */
class PointerType : public SequentialType {
public:
  PointerType(std::string type_str);
  virtual ~PointerType();
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var, bool simple=false) override;
  virtual void GenerateIOFunc() override;
  virtual std::string GetOutputCode(std::string var, bool simple=false) override;
  virtual InputSpec *GenerateRandomInput(bool simple=false) override;
  virtual std::vector<InputSpec*> GeneratePairInput() override;
  virtual std::string GetRaw() override {
    std::string ret;
    if (m_contained_type) {
      ret += m_contained_type->GetRaw() + "*";
    }
    return ret;
  }
  virtual std::string ToString() override {
    std::string ret = "PointerType: ";
    if (m_contained_type) {
      ret += m_contained_type->ToString();
    } else {
      ret += "NULL";
    }
    return ret;
  }
  void SetRaw(std::string raw) {m_raw=raw;}
  void SetContainedType(Type *t) {m_contained_type=t;}
protected:
private:
  Type *m_contained_type = NULL;
  // used to set the raw.
  // if this is not empty, the *decl* code will use this plus the variable name
  std::string m_raw;
};

// class StrType : public PointerType {
// public:
//   StrType();
//   virtual ~StrType();
//   virtual std::string GetDeclCode(std::string var) override;
//   virtual std::string GetInputCode(std::string var) override;
//   virtual std::string GetOutputCode(std::string var) override;
//   virtual InputSpec *GenerateRandomInput() override;
//   virtual std::vector<InputSpec*> GeneratePairInput() override;
//   virtual std::string ToString() override {
//     return "StrType";
//   }
// protected:
// private:
//   InputSpec* wrap(std::string s);
// };

// class BufType : public ArrayType {
// public:
//   BufType(int num);
//   virtual ~BufType();
//   virtual std::string GetDeclCode(std::string var) override;
//   virtual std::string GetInputCode(std::string var) override;
//   virtual std::string GetOutputCode(std::string var) override;
//   virtual InputSpec *GenerateRandomInput() override;
//   virtual std::string ToString() override {return "BufType";}
// protected:
// private:
// };



/**
 * Primitives
 */
class IntType : public PrimitiveType {
public:
  IntType();
  ~IntType();
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var, bool simple=false) override;
  virtual std::string GetOutputCode(std::string var, bool simple=false) override;
  virtual InputSpec *GenerateRandomInput(bool simple=false) override;
  virtual std::string GetRaw() override {return "int";}
  virtual std::vector<InputSpec*> GeneratePairInput() override;
  virtual std::string ToString() override {return "IntType";}
protected:
private:
  // wrap value into a InputSpec
  InputSpec *wrap(int value);
};

class CharType : public PrimitiveType {
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var, bool simple=false) override;
  virtual std::string GetOutputCode(std::string var, bool simple=false) override;
  virtual InputSpec *GenerateRandomInput(bool simple=false) override;
  virtual std::string GetRaw() override {return "char";}
  virtual std::vector<InputSpec*> GeneratePairInput() override;
  virtual std::string ToString() override {return "CharType";}
public:
  CharType();
  virtual ~CharType();
private:
  InputSpec* wrap(char c);
};

class BoolType : public PrimitiveType {
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var, bool simple=false) override;
  virtual std::string GetOutputCode(std::string var, bool simple=false) override;
  virtual InputSpec *GenerateRandomInput(bool simple=false) override;
  virtual std::string GetRaw() override {return "bool";}
  virtual std::string ToString() override {return "BoolType";}
public:
  BoolType();
  virtual ~BoolType();
private:
};



















// PointerType *make_struct_pointer_type(int id, int level);


/**
 * Special type
 */

// class ArgCVType : public SpecialType {
// public:
//   ArgCVType(std::string getopt_str="");
//   virtual ~ArgCVType();
//   virtual std::string GetDeclCode(std::string var) override;
//   virtual std::string GetInputCode(std::string var) override;
//   virtual std::string GetOutputCode(std::string var) override;
//   virtual InputSpec *GenerateRandomInput() override;
//   virtual std::string GetRaw() override {
//     return "";
//   }
//   virtual std::string ToString() override {
//     return "ArgCVType";
//   }
// private:
//   std::vector<char> m_bools;
//   std::vector<char> m_named_args;
//   Type *m_argv;
//   Type *m_argc;
// };

// std::vector<std::pair<InputSpec*, InputSpec*> > pairwise(Type* a, Type *b);

#endif /* TYPE_H */
