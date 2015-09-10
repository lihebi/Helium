#ifndef __SYSTEM_TYPE_HPP__
#define __SYSTEM_TYPE_HPP__

#include "type/Type.hpp"

class SystemType : public Type {
public:
  virtual void GetInputCode();
  virtual void GetOutputCode();
  virtual void GetInputSpecification();
  virtual void GetOutputSpecification();
private:
};

#endif
