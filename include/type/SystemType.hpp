#ifndef __SYSTEM_TYPE_HPP__
#define __SYSTEM_TYPE_HPP__

#include "type/Type.hpp"
#include "type/TypeFactory.hpp"

class SystemType : public Type {
public:
  SystemType(const std::string& name, const struct struct_specifier& specifier);
  ~SystemType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual std::string GetInputSpecification();
  virtual std::string GetOutputSpecification();
  virtual std::string GetName() const { return m_name;}
private:
  std::string m_name;
  struct struct_specifier m_specifier;
  std::string m_type;
};

#endif
