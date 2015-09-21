#ifndef __SYSTEM_TYPE_HPP__
#define __SYSTEM_TYPE_HPP__

#include "type/Type.hpp"

class SystemType : public Type {
public:
  SystemType(const std::string& name);
  ~SystemType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual void GetInputSpecification();
  virtual void GetOutputSpecification();
  virtual std::string GetName() const { return m_name;}
private:
  std::string m_name;
};

#endif
