#ifndef TYPE_H
#define TYPE_H

#include "common.h"
#include "parser/xmlnode.h"
#include "utils/string_utils.h"
#include "input_spec.h"

class Type;
class TypeFactory {
public:
  static Type *CreateType(XMLNode decl_node);
  static Type *CreateType(std::string str);
};

class Type {
public:
  Type() {}
  virtual ~Type() {}
  virtual InputSpec* GenerateRandomInput() = 0;
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
  virtual std::string GetInputCode(std::string var) = 0;
  virtual std::string GetOutputCode(std::string var) = 0;
  virtual void GenerateIOFunc() = 0;
  virtual std::vector<InputSpec*> GenerateCornerInputs(int limit=-1);
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

class ConstType : public Type {
public:
  ConstType(std::string type_str) {
    // assert type_str don't have "const"
    // FIXME the structure type name might contain const
    // assert(type_str.find("const") == std::string::npos);
    m_contained_type = TypeFactory::CreateType(type_str);
    assert(m_contained_type);
    // assert m_contained_type is not null
  }
  virtual ~ConstType() {}
private:
  virtual std::string GetDeclCode(std::string var) override {
    // return "const " + m_contained_type->GetDeclCode(var);
    // FIXME I didn't add anything to the decl. I think decl it as a non-const is fine
    // I cannot add anything because I added some comments before the decl, like // IntType
    return m_contained_type->GetDeclCode(var);
  }
  virtual std::string GetInputCode(std::string var) override {
    return m_contained_type->GetInputCode(var);
  }
  virtual void GenerateIOFunc() override {
    // FIXME the IO func must have const!!
    m_contained_type->GenerateIOFunc();
  }
  virtual std::string GetOutputCode(std::string var) override {
    return m_contained_type->GetOutputCode(var);
  }
  virtual InputSpec *GenerateRandomInput() override {
    return m_contained_type->GenerateRandomInput();
  }
  virtual std::vector<InputSpec*> GeneratePairInput() override {
    return m_contained_type->GeneratePairInput();
  }
  virtual std::string GetRaw() override {
    return "const " + m_contained_type->GetRaw();
  }
  virtual std::string ToString() override {
    std::string ret = "ConstType: ";
    if (m_contained_type) {
      ret += m_contained_type->ToString();
    } else {
      ret += "NULL";
    }
    return ret;
  }
  Type *m_contained_type;
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


class UnknownType : public Type {
public:
  UnknownType(std::string str) : m_raw(str) {}
  virtual ~UnknownType() {}
  virtual InputSpec* GenerateRandomInput() override { return NULL;}
  virtual std::string GetDeclCode(std::string var) override {
    std::string ret;
    ret += "// UnknownType::GetDeclCode: " + var + ";\n";
    ret += m_raw + " " + var + ";\n";
    return ret;
  }
  virtual void GenerateIOFunc() override {};
  virtual std::string GetInputCode(std::string var) override {
    var.empty();
    return "";
  }
  virtual std::string GetOutputCode(std::string var) override {
    var.empty();
    return "";
  }
  virtual std::string GetRaw() override {return m_raw;}
  virtual std::string ToString() override {
    return "UnknownType: " + m_raw;
  }
private:
  std::string m_raw;
};


#endif /* TYPE_H */
