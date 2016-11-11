#ifndef SEQUENTIAL_TYPE_H
#define SEQUENTIAL_TYPE_H

#include "type.h"

/**
 * Second level
 */

class SequentialType : public Type {
public:
  SequentialType() {}
  virtual ~SequentialType() {}
protected:
private:
};


/**
 * ArrayType
 * When generating input, need to look into contained type to see if that is a "char".
 */
class ArrayType : public SequentialType {
public:
  ArrayType(std::string type_str, int num);
  ~ArrayType();
  virtual std::string GetDeclCode(std::string var) override;
  virtual void GenerateIOFunc() override;
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




#endif /* SEQUENTIAL_TYPE_H */
