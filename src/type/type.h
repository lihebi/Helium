#ifndef TYPE_H
#define TYPE_H

#include "common.h"
#include "parser/xmlnode.h"

/**
 * The spec for a chain of type is hard. But I have solution.
 * The spec format is a like-"json" format: use {} for the level.
 * For a struct, it will be {.field1 xxx, .field2 xxx}
 * When passing to the outer level, it becomes:
 * {.out_field1 {.field1 xxx}, ...}
 * finally for the outmost type, we have the variable name.
 */
class InputSpec {
public:
  InputSpec();
  ~InputSpec();
  void Add(InputSpec *spec);
  void SetRaw(std::string raw) {
    m_raw = raw;
  }
  void SetSpec(std::string spec) {
    m_spec = spec;
  }
private:
  std::string m_raw;
  std::string m_spec;
};

class Type;
class TypeFactory {
public:
  static Type *CreateType(XMLNode decl_node);
};

class Type {
public:

  virtual InputSpec* GenerateInput();
  
  virtual std::string GetDeclCode(std::string var);
  virtual std::string GetInputCode(std::string var);
  virtual std::string GetOutputCode(std::string var);

  // Get the "raw" of the type, e.g. "char"
  // this is proposed for "char* xx = malloc(sizeof(char) * num)". Note that sizeof(char)
  virtual std::string GetValue();
protected:
  friend class TypeFactory;
  Type();
  virtual ~Type();
private:
};

/**
 * First level
 */
class SystemType : public Type {
public:
protected:
  SystemType();
  ~SystemType();
private:
};

class PrimitiveType : public Type {
public:
protected:
  PrimitiveType();
  ~PrimitiveType();
private:
};

class CompositeType : public Type {
public:
protected:
  CompositeType();
  ~CompositeType();
private:
};

class SpecialType : public Type {
public:
protected:
  SpecialType();
  ~SpecialType();
private:
};

/**
 * Second level
 */

class SequentialType : public CompositeType {
public:
protected:
  SequentialType();
  ~SequentialType();
private:
};

class StructType : public CompositeType {
public:
protected:
  StructType();
  ~StructType();
private:
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
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual InputSpec *GenerateInput() override;
protected:
  ArrayType();
  ~ArrayType();
private:
  Type *m_contained_type = NULL;
  int m_num = 0;
};

/**
 * Pointer type.
 * When generating input, need to look into contained type to see if that is a "char".
 */
class PointerType : public SequentialType {
public:
  virtual std::string GetDeclCode(std::string var) override;
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual InputSpec *GenerateInput() override;
protected:
  PointerType();
  ~PointerType();
private:
  Type *m_contained_type = NULL;
};



/**
 * Primitives
 */
class IntType : public PrimitiveType {
public:
protected:
  IntType();
  ~IntType();
private:
};

class CharType : public PrimitiveType {
public:
  CharType();
  ~CharType();
private:
};


/**
 * Special type
 */

class ArgCVType : public SpecialType {
public:
  ArgCVType();
  ~ArgCVType();
private:
};



#endif /* TYPE_H */
