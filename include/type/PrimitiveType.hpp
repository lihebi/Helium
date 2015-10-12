#ifndef __PRIMITIVE_TYPE_HPP__
#define __PRIMITIVE_TYPE_HPP__

#include <string>
#include <set>

#include "type/Type.hpp"
#include "type/TypeFactory.hpp"
#include <iostream>

class PrimitiveType : public Type {
public:
  // Deprecated
  PrimitiveType(const struct type_specifier& specifier);
  virtual ~PrimitiveType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual std::string GetInputSpecification();
  virtual std::string GetOutputSpecification();
  virtual std::string GetName() const {return m_name;}

private:

  struct type_specifier m_specifier;

  std::string m_name;
};

#endif
