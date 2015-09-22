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
  virtual std::string GetName() const { return m_name;}
  void SetDimension(int d) {m_dimension = d;}
  void SetPointerLevel(int l) {m_pointer_level = l;}
private:
  int m_dimension;
  int m_pointer_level;
  std::string m_name;
};

#endif
