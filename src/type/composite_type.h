#ifndef COMPOSITE_TYPE_H
#define COMPOSITE_TYPE_H

#include "type.h"

class CompositeType : public Type {
public:
  CompositeType() {}
  virtual ~CompositeType() {}
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



#endif /* COMPOSITE_TYPE_H */
