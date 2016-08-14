#ifndef TYPE_H
#define TYPE_H

#include "common.h"
#include "parser/xmlnode.h"

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
  virtual InputSpec* GenerateInput() = 0;
  virtual std::string GetDeclCode(std::string var) = 0;
  virtual std::string GetInputCode(std::string var) = 0;
  virtual std::string GetOutputCode(std::string var) = 0;

  // Get the "raw" of the type, e.g. "char"
  // this is proposed for "char* xx = malloc(sizeof(char) * num)". Note that sizeof(char)
  // TODO what's this?
  // virtual std::string GetValue() = 0;
  virtual std::string GetRaw() = 0;
protected:
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

class SpecialType : public Type {
public:
  SpecialType() {}
  virtual ~SpecialType() {}
protected:
private:
};

class UnknownType : public Type {
public:
  UnknownType(std::string str) : m_raw(str) {}
  virtual ~UnknownType() {}
  virtual InputSpec* GenerateInput() { return NULL;}
  virtual std::string GetDeclCode(std::string var) {
    std::string ret;
    ret += "// UnknownType::GetDeclCode: " + var + ";\n";
    if (m_raw.find('[') != std::string::npos) {
      std::string prefix = m_raw.substr(0, m_raw.find('['));
      std::string suffix = m_raw.substr(m_raw.find('['));
      ret += prefix + " " + var + suffix + ";\n";
    } else {
      ret += m_raw + " " + var;
    }
    return ret;
  }
  virtual std::string GetInputCode(std::string var) {
    var.empty();
    return "";
  }
  virtual std::string GetOutputCode(std::string var) {
    var.empty();
    return "";
  }
  virtual std::string GetRaw() {return m_raw;}
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
  StructType() {}
  virtual ~StructType() {}
protected:
private:
  // int m_snippet_id = -1;
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
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual InputSpec *GenerateInput() override;
  virtual std::string GetRaw() override {
    std::string ret;
    if (m_contained_type) {
      ret += m_contained_type->GetRaw() + "[" + std::to_string(m_num) + "]";
    }
    return ret;
  }
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
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual InputSpec *GenerateInput() override;
  virtual std::string GetRaw() override {
    std::string ret;
    if (m_contained_type) {
      ret += m_contained_type->GetRaw() + "*";
    }
    return ret;
  }
protected:
private:
  Type *m_contained_type = NULL;
};

/**
 * StrType, aka char* and char[].
 * Not sure if I need a "buffer type" for char[]
 * This is created when construct Type.
 * I.e. the char* should NOT be recognized as PointerType, but StrType
 * Super clean!
 * I'm genius.
 */
class StrType : public PointerType {
public:
  StrType();
  virtual ~StrType();
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual InputSpec *GenerateInput() override;
protected:
private:
};

class BufType : public ArrayType {
public:
  BufType(int num);
  virtual ~BufType();
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual InputSpec *GenerateInput() override;
protected:
private:
};



/**
 * Primitives
 */
class IntType : public PrimitiveType {
public:
  IntType();
  ~IntType();
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual InputSpec *GenerateInput() override;
  virtual std::string GetRaw() override {return "int";}
protected:
private:
};

class CharType : public PrimitiveType {
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual InputSpec *GenerateInput() override;
  virtual std::string GetRaw() override {return "char";}
public:
  CharType();
  virtual ~CharType();
private:
};

class BoolType : public PrimitiveType {
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual InputSpec *GenerateInput() override;
  virtual std::string GetRaw() override {return "bool";}
public:
  BoolType();
  virtual ~BoolType();
private:
};


/**
 * Special type
 */

class ArgCVType : public SpecialType {
public:
  ArgCVType(std::string getopt_str="");
  virtual ~ArgCVType();
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual InputSpec *GenerateInput() override;
  virtual std::string GetRaw() override {
    return "";
  }
private:
  std::vector<char> m_bools;
  std::vector<char> m_named_args;
  Type *m_argv;
  Type *m_argc;
};



#endif /* TYPE_H */
