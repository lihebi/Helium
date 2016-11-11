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


class UnknownType : public Type {
public:
  UnknownType(std::string str) : m_raw(str) {}
  virtual ~UnknownType() {}
  virtual InputSpec* GenerateRandomInput(bool simple=false) override { return NULL;}
  virtual std::string GetDeclCode(std::string var) override {
    std::string ret;
    ret += "// UnknownType::GetDeclCode: " + var + ";\n";
    ret += m_raw + " " + var + ";\n";
    return ret;
  }
  virtual void GenerateIOFunc() override {};
  virtual std::string GetInputCode(std::string var, bool simple=false) override {
    var.empty();
    return "";
  }
  virtual std::string GetOutputCode(std::string var, bool simple=false) override {
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
