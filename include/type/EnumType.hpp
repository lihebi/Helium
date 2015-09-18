#ifndef __ENUM_TYPE_HPP__
#define __ENUM_TYPE_HPP__

#include "type/Type.hpp"

class EnumType : public Type {
public:
  EnumType(const std::string& name);
  virtual ~EnumType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual void GetInputSpecification();
  virtual void GetOutputSpecification();
private:
  std::string m_name;
};

#endif
