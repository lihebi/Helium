#ifndef __ENUM_TYPE_HPP__
#define __ENUM_TYPE_HPP__

#include "type/Type.hpp"

class EnumType : public Type {
public:
  EnumType();
  virtual ~EnumType();
  virtual void GetInputCode();
  virtual void GetOutputCode();
  virtual void GetInputSpecification();
  virtual void GetOutputSpecification();
private:
};

#endif
