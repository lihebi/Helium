#ifndef PRIMITIVE_TYPE_H
#define PRIMITIVE_TYPE_H

#include "type.h"

class PrimitiveType : public Type {
public:
  PrimitiveType() {}
  virtual ~PrimitiveType() {}
  virtual void GenerateIOFunc() override {}
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
  virtual InputSpec *GenerateRandomInput() override;
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
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual InputSpec *GenerateRandomInput() override;
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
  virtual std::string GetInputCode(std::string var) override;
  virtual std::string GetOutputCode(std::string var) override;
  virtual InputSpec *GenerateRandomInput() override;
  virtual std::string GetRaw() override {return "bool";}
  virtual std::string ToString() override {return "BoolType";}
public:
  BoolType();
  virtual ~BoolType();
private:
};




#endif /* PRIMITIVE_TYPE_H */
