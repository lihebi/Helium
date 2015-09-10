#ifndef __STRUCTURE_TYPE_HPP__
#define __STRUCTURE_TYPE_HPP__

#include "type/Type.hpp"

class StructureType : public Type {
public:
  StructureType();
  virtual ~StructureType();
  virtual void GetInputCode();
  virtual void GetOutputCode();
  virtual void GetInputSpecification();
  virtual void GetOutputSpecification();
private:
  void simplifyCode();
};

#endif
