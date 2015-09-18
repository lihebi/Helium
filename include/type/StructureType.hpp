#ifndef __STRUCTURE_TYPE_HPP__
#define __STRUCTURE_TYPE_HPP__

#include "type/Type.hpp"

class StructureType : public Type {
public:
  StructureType(const std::string& name);
  virtual ~StructureType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual void GetInputSpecification();
  virtual void GetOutputSpecification();
private:
  void simplifyCode();
  std::string m_name;
};

#endif
