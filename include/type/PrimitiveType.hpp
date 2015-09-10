#ifndef __PRIMITIVE_TYPE_HPP__
#define __PRIMITIVE_TYPE_HPP__

#include <string>

#include "type/Type.hpp"

class PrimitiveType : public Type {
public:
  PrimitiveType();
  virtual ~PrimitiveType();
  virtual void GetInputCode();
  virtual void GetOutputCode();
  virtual void GetInputSpecification();
  virtual void GetOutputSpecification();

private:
  std::string m_name;
};

#endif
