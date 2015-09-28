#ifndef __PRIMITIVE_TYPE_HPP__
#define __PRIMITIVE_TYPE_HPP__

#include <string>
#include <set>

#include "type/Type.hpp"

class PrimitiveType : public Type {
public:
  // Deprecated
  PrimitiveType(const std::string& name);
  PrimitiveType(uint8_t length, uint8_t type);
  virtual ~PrimitiveType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual void GetInputSpecification();
  virtual void GetOutputSpecification();
  virtual std::string GetName() const {return m_name;}
  void SetDimension(int d) {m_dimension = d;}
  void SetPointerLevel(int l) {m_pointer_level = l;}
private:

  std::string getIntInputCode(const std::string& var) const;
  std::string getCharInputCode(const std::string& var) const;
  std::string getVoidInputCode(const std::string& var) const;

  int m_dimension;
  int m_pointer_level;
  std::string m_name;
  uint8_t m_length;
  uint8_t m_type;
};

#endif
